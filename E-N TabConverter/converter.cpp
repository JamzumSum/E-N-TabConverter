#include "stdafx.h"
#include "Dodo.h"
#include "eagle.h"
#include "global.h"
#include "Cuckoo.h"
#include "swan.h"
#include "tools.h"
#include "converter.h"
#include <numeric>

//vcpkg include
#include <baseapi.h>

using std::get;
using std::thread;

thread_local const Converter* root;

void Converter::Train() {
	CharReader::train(prop);
}

const std::tuple<int, int, int> Converter::cvVersion() { return {CV_VERSION_MAJOR, CV_VERSION_MINOR, CV_VERSION_REVISION}; }
const std::tuple<int, int, int> Converter::Version() { return { EN_VERSION_MAJOR, EN_VERSION_MINOR, EN_VERSION_REVISON }; }
const std::tuple<int, int, int> Converter::TinyXMLVersion() { return { TINYXML2_MAJOR_VERSION, TINYXML2_MINOR_VERSION, TINYXML2_PATCH_VERSION }; };

void Converter::title(const vector<Mat>& info) {
	Mat ccolor;
	cvtColor(info[0], ccolor, CV_GRAY2BGR);
	while (!ocrReady) std::this_thread::yield();

	auto ocr = reinterpret_cast<tesseract::TessBaseAPI*>(api);
	ocr->SetImage(info[0].data, info[0].cols, info[0].rows, ccolor.channels(), static_cast<int>(ccolor.step));

	char* title = ocr->GetUTF8Text();
	auto caption = Utf8ToGbk(title);
	delete[] title;
	api = nullptr;
	string composer, lyricist, artist, tabber, irights;

	doc = new saveDoc(caption, composer, lyricist, artist, tabber, irights);
	ocr->End();
}

Converter::Converter(const vector<string>& pics): api(new tesseract::TessBaseAPI()) {
	root = this;
	using namespace tesseract;
	assert(!pics.empty());
	picPath = pics;
	if constexpr (TesseractEnabled) {
		thread([this]() {
			assert(reinterpret_cast<TessBaseAPI*>(api)->TessBaseAPI::Init("D:\\Program Files\\Tesseract-OCR\\tessdata", "chi_sim+eng") == 0);
			ocrReady = true;
		}).detach();
	}
}

auto Converter::scan (const int startWith, string picPath, function<void(string)> notify, function<void(int)> progress) {
	bool flag = false;
	Mat img = cv::imread(picPath.c_str(), 0);
	if (img.empty()) raiseErr("Wrong format.", 3);
	Mat trimmed = threshold(img);
	
	trimmed = trim(trimmed);
	auto dpi = getScaleFactor();
	auto screen = getWorkspaceSize();
	float screenCols = screen.first / dpi.first;
	
	if (trimmed.cols > screenCols) {
		float toRows = screenCols / trimmed.cols * trimmed.rows;
		trimmed = perspect(trimmed, int(screenCols), int(toRows));
		trimmed = threshold(trimmed);
	}
	global.setCol(trimmed.cols);
	vector<Mat> piece;
	
	Splitter piccut(trimmed);
	piccut.start(piece);
	
	progress(1);
	notify("过滤算法正常");
	
	int n = static_cast<int>(piece.size());
	vector<Mat> info;							//其余信息
	
	using general = std::tuple<vector<Vec4i>, vector<int>, vector<Vec4i>, Mat>;
	vector<general> sectionsGrid; sectionsGrid.reserve(n);

	std::atomic_int cnt = 0;
	auto scanGrid = [this, &cnt, &sectionsGrid, &n, &info](Mat i, int index) {
		root = this;
		vector<Vec4i> rows;
		LineFinder finder(i, 10);
		finder.findRow(rows);
		if (rows.size() == 6) {
			vector<Vec4i> lines;
			finder.findCol(lines);
			while (cnt < index - 1) std::this_thread::yield();
			sectionsGrid.emplace_back(rows, finder.getLineThickness(), lines, i);
			n += static_cast<int>(lines.size());
			cnt++;
		}
		else {
			while (cnt < index - 1) std::this_thread::yield();
			info.push_back(i);
			cnt++;
		}
	};
	int tmpsize = static_cast<int>(n);
	n = 0;
	for (int k = 0; k < tmpsize; k++) {
		std::thread t(scanGrid, piece[k], k + 1);
		t.detach();						//race!
	}
	while (cnt < tmpsize) {
		std::this_thread::yield();
	}
	vector<Mat>().swap(piece); sectionsGrid.shrink_to_fit();
	progress(30);

	reader = new knnReader();
	vector<const Measure*> sections;			//按行存储
	sections.reserve(n);
	cnt = 0;
	std::atomic_int cnt2 = 0;
	tmpsize = static_cast<int>(sectionsGrid.size());
	auto scanMeasure = [this, &cnt, &cnt2, &sections, &progress, tmpsize](const general& i, int index) {
		root = this;
		vector<Mat> origin;
		if (!std::get<1>(i).empty()) cut(get<3>(i), get<2>(i), 0, origin);
		int avrWidth = global["colLenth"];

		origin.erase(remove_if(origin.begin(), origin.end(), 
			[avrWidth](const Mat& x) {return x.cols < avrWidth / 5; }), origin.end());
		
		const_cast<GlobalUnit&>(global["rowLenth"]) += get<3>(i).rows;

		while (cnt < index) std::this_thread::yield();
		size_t pre = sections.size();
		for (size_t j = 0; j < origin.size(); j++) {
			sections.emplace(sections.begin() + pre + j, new Measure(origin[j], pre + j + 1));
		}
		cnt++;

		for (size_t j = 0; j < origin.size(); j++) {
			const_cast<Measure*>(sections[pre + j])->start(get<0>(i), get<1>(i));
		}
		cnt2++;
		progress(cnt2 / tmpsize / 2 + 30);
	};
	for (int i = 0; i < tmpsize; i++) {
		thread t(scanMeasure, ref(sectionsGrid[i]), i);
		t.join();
	}
	while (cnt2 < tmpsize) std::this_thread::yield();
	vector<Mat>().swap(piece);

	sections.erase(remove_if(sections.begin(), sections.end(), [](const Measure* x) -> bool {return x->empty(); }),
		sections.end());

	for (auto& i : sections) const_cast<Measure*>(i)->setID(startWith + i->ID());

	if constexpr (TesseractEnabled) {
		if (startWith == 0) {
			title(info);
		}
	}

	return sections;
}

string Converter::scan(function<void(string)> notify, function<void(int)> progress) {
	int n = static_cast<int>(picPath.size());
	size_t ms = 0;
	vector<int> prog(n);
	vector<vector<const Measure*>> ptr(n);
	std::atomic_int cnt = 0;
	for (int i = 0; i < n; i++) {
		ptr[i] = scan(static_cast<int>(ms), picPath[i], notify,
			[i, n, &prog, &progress](int p) {
				prog[i] = p; progress(accumulate(prog.begin(), prog.end(), 0) / n);
			});
		ms += ptr[i].size();
	}
	vector<int>().swap(prog);

	notify("扫描完成，准备写入文件");
	progress(80);
	global.save();
	string name = fname(picPath[0]);
	size_t i = 0;
	if (!doc) {
		doc = new saveDoc(name, "unknown", "unknown", "unknown", PROJECT, "Internet");
	}
	saveDoc* finish = (saveDoc*)doc;

	while (cnt) std::this_thread::yield();

	for (const auto& v : ptr) {
		for (const auto m : v) {
			assert(m->ID() != 0);
			finish->saveMeasure(m->getNotes());
			progress(static_cast<int>(80 + i++ / ms));
		}
	}

	for (const auto& v : ptr) for (const auto m : v) delete m;

	string fn = outputDir;
	if (fn.back() != '\\') fn.append("\\");
	fn.append(name).append(".musicxml");
	if (isExist(fn)) if (prompt(NULL, fn + " 已经存在，要替换吗？", PROJECT, 0x34) == 7) {
		assert(selectSaveStrategy);
		fn = selectSaveStrategy();
		if (fn.empty()) {
			notify("用户放弃了保存. ");
			return string();
		}
		if (fn.back() != '\\') fn.append("\\");
		fn.append(name).append(".musicxml");
	}
	if (auto xmlret = finish->save(fn); 0 ==  xmlret) {
		progress(100);
		notify("Success: " + name + ".musicxml");
	}
	else notify("Error when saving doc, error code " + xmlret);
	cv::destroyAllWindows();
	if (doc) {
		delete doc;
		doc = nullptr;
	}
	return name + ".musicxml";
}

Converter::~Converter() {
	if (doc) {
		delete reinterpret_cast<saveDoc*>(doc);
		doc = nullptr;
	}
	if (api) {
		reinterpret_cast<tesseract::TessBaseAPI*>(api)->End();
		api = nullptr;
	}
	if (reader) {
		delete reader;
		reader = nullptr;
	}
}
