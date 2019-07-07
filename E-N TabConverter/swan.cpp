#include "stdafx.h"
#include "swan.h"
#include "type.h"
#include "tools.h"

using namespace tinyxml2;
using namespace std;

#define ele(x) XMLElement* x = doc.NewElement(#x)
#define txtx(x) XMLText* x##Text = doc.NewText(x)
#define txts(x,s) XMLText* x##Text = doc.NewText(#s)
#define txt(x) txts(x,x)

static char* pitch(int string,int fret,const char* tuning,char* r,bool & up) {
	char law[13] = "CCDDEFFGGAAB";
	// 1 3 6 8 10
	int i;
	for (i = 0; i < 13; i++) {
		if (law[i] == tuning[0]) break;
	}
	//此时i是音调的位置
	int m = (i + fret) % 12;
	r[0] = law[m];
	r[1] = tuning[1] + (i + fret) / 12;
	r[2] = 0;
	switch (m)
	{
	case 1:
	case 3:
	case 6:
	case 8:
	case 10:
		up = true;
		break;
	default:
		up = false;
		break;
	}
	return r;
}

static void CopyNode(tinyxml2::XMLDocument *desdoc, const tinyxml2::XMLDocument *srcdoc)
{
	// Protect from evil
	if (desdoc == NULL || srcdoc == NULL)
	{
		return;
	}
	for (const XMLNode* node = srcdoc->FirstChild(); node; node = node->NextSibling()) {
		XMLNode* copy = node->DeepClone(desdoc);
		desdoc->InsertEndChild(copy);
	}
	
}

saveDoc::saveDoc(const string& title, const string& composer, const string& lyricist, const string& artist, const string& tabber, const string& irights)
{
	tinyxml2::XMLDocument doc;
	doc.Parse("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 2.0 Partwise//EN' 'http://www.musicxml.org/dtds/2.0/partwise.dtd'>");
	XMLElement* root = doc.NewElement("score-partwise");
	doc.InsertEndChild(root);
	root->SetAttribute("version","3.0");

	ele(work);
	XMLElement* workTitle = doc.NewElement("work-title");

	string UTF8Title = GBKToUTF8(title);
	work->InsertEndChild(workTitle);
	workTitle->InsertEndChild(doc.NewText(UTF8Title.c_str()));
	root->InsertEndChild(work);

	ele(identification);

	XMLElement* creator_composer = doc.NewElement("creator");
	creator_composer->SetAttribute("type", "composer");
	XMLText* composerText = doc.NewText(composer.c_str());
	creator_composer->InsertEndChild(composerText);

	XMLElement* creator_lyricist = doc.NewElement("lyricist");
	creator_lyricist->SetAttribute("type", "lyricist");
	XMLText* lyricistText = doc.NewText(lyricist.c_str());
	creator_lyricist->InsertEndChild(lyricistText);

	XMLElement* creator_artist = doc.NewElement("artist");
	creator_artist->SetAttribute("type", "artist");
	XMLText* artistText = doc.NewText(artist.c_str());
	creator_artist->InsertEndChild(artistText);

	XMLElement* creator_tabber = doc.NewElement("tabber");
	creator_tabber->SetAttribute("type", "tabber");
	XMLText* tabberText = doc.NewText(tabber.c_str());
	creator_tabber->InsertEndChild(tabberText);

	ele(rights);
	XMLText* irightsText = doc.NewText(irights.c_str());
	rights->InsertEndChild(irightsText);

	ele(encoding);
	ele(software);
	XMLText* GP = doc.NewText("Guitar Pro 7");
	software->InsertEndChild(GP);
	encoding->InsertEndChild(software);

	identification->InsertEndChild(creator_tabber);
	identification->InsertEndChild(creator_lyricist);
	identification->InsertEndChild(creator_artist);
	identification->InsertEndChild(creator_tabber);
	identification->InsertEndChild(rights);
	identification->InsertEndChild(encoding);
	root->InsertEndChild(identification);

	XMLElement* part_list = doc.NewElement("part-list");
	root->InsertEndChild(part_list);
	XMLElement* score_part = doc.NewElement("score-part");
	part_list->InsertEndChild(score_part);
	score_part->SetAttribute("id", "p0");
	XMLElement* part_name = doc.NewElement("part-name");
	XMLText* G = doc.NewText("Guitar");
	part_name->InsertEndChild(G);
	score_part->InsertEndChild(part_name);

	XMLElement* part_abbreviation = doc.NewElement("part-abbreviation");
	XMLText* S_Gt = doc.NewText("s.guit.");
	part_abbreviation->InsertEndChild(S_Gt);
	score_part->InsertEndChild(part_abbreviation);

	XMLElement* score_instrument = doc.NewElement("score-instrument");
	XMLElement* instrument_name = doc.NewElement("instrument-name");
	score_instrument->SetAttribute("id", "i0");
	score_part->InsertEndChild(score_instrument);
	score_instrument->InsertEndChild(instrument_name);

	XMLElement* midi_instrument = doc.NewElement("midi-instrument");
	midi_instrument->SetAttribute("id", "i0");
	
	
	XMLElement* midi_channel = doc.NewElement("midi-channel");
	XMLElement* midi_program = doc.NewElement("midi-program");
	ele(volume);
	ele(pan);
	txts(midi_program, 26);
	midi_channel->InsertEndChild(doc.NewText("1"));
	midi_program->InsertEndChild(midi_programText);
	volume->InsertEndChild(doc.NewText("80"));
	pan->InsertEndChild(doc.NewText("0"));
	midi_instrument->InsertEndChild(midi_channel);
	midi_instrument->InsertEndChild(midi_program);
	midi_instrument->InsertEndChild(volume);
	midi_instrument->InsertEndChild(pan);
	score_part->InsertEndChild(midi_instrument);

	XMLElement* part = doc.NewElement("part");
	part->SetAttribute("id", "p0");
	root->InsertEndChild(part);
	CopyNode(&backup, &doc);
}
int saveDoc::save(string xmlPath) {
	return backup.SaveFile(xmlPath.c_str());
}
void saveDoc::saveMeasure(MusicMeasure toSave) {
	tinyxml2::XMLDocument doc;
	CopyNode(&doc,&backup);
	XMLElement* part = doc.FirstChildElement("score-partwise")->FirstChildElement("part");
	XMLElement* measure = doc.NewElement("measure");
	part->InsertEndChild(measure);
	measure->SetAttribute("number", int(toSave.id));

	ele(attributes);
	XMLElement* divisions = doc.NewElement("divisions");
	XMLText* l024 = doc.NewText("1024");
	divisions->InsertEndChild(l024);
	attributes->InsertEndChild(divisions);

	XMLElement* key = doc.NewElement("key");
	XMLElement* fifths = doc.NewElement("fifths");
	XMLElement* mode = doc.NewElement("mode");
	
	char a[5];
	_itoa_s(toSave.key.fifth, a, 10);
	XMLText* fifthsText = doc.NewText(a);
	XMLText* modeText = doc.NewText(toSave.key.mode.c_str());
	fifths->InsertEndChild(fifthsText);
	mode->InsertEndChild(modeText);
	key->InsertEndChild(fifths);
	key->InsertEndChild(mode);
	attributes->InsertEndChild(key);

	XMLElement* time = doc.NewElement("time");
	XMLElement* beats = doc.NewElement("beats");
	XMLElement* beat_type = doc.NewElement("beat-type");
	_itoa_s(toSave.time.beats, a, 10);
	XMLText* beatsText = doc.NewText(a);
	_itoa_s(toSave.time.beat_type, a, 10);
	XMLText* beat_typeText = doc.NewText(a);
	beats->InsertEndChild(beatsText);
	beat_type->InsertEndChild(beat_typeText);
	time->InsertEndChild(beats);
	time->InsertEndChild(beat_type);
	attributes->InsertEndChild(time);

	attributes->InsertEndChild(doc.NewElement("staves"))->InsertEndChild(doc.NewText("2"));
	ele(clef);
	ele(sign);
	ele(line);
	XMLElement* clef2 = doc.NewElement("clef");
	attributes->InsertEndChild(clef2);
	clef2->InsertEndChild(doc.NewElement("sign"))->InsertEndChild(doc.NewText("G"));
	clef2->InsertEndChild(doc.NewElement("line"))->InsertEndChild(doc.NewText("2"));
	clef2->SetAttribute("number", "1");

	sign->InsertEndChild(doc.NewText("TAB"));
	line->InsertEndChild(doc.NewText("5"));
	attributes->InsertEndChild(clef);
	clef->SetAttribute("number","2");
	clef->InsertEndChild(sign);
	clef->InsertEndChild(line);
	

	XMLElement* staff_details = doc.NewElement("staff-details");
	XMLElement* staff_lines = doc.NewElement("staff-lines");
	staff_lines->InsertEndChild(doc.NewText("6"));
	//标准调弦

	XMLElement* staff_tuning1 = doc.NewElement("staff-tuning");
	staff_tuning1->SetAttribute("line", "1");
	XMLElement* tuning_step1 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave1 = doc.NewElement("tuning-octave");
	tuning_step1->InsertEndChild(doc.NewText("E"));
	tuning_octave1->InsertEndChild(doc.NewText("2"));					//E2
	staff_tuning1->InsertEndChild(tuning_step1);
	staff_tuning1->InsertEndChild(tuning_octave1);

	XMLElement* staff_tuning2 = doc.NewElement("staff-tuning");
	staff_tuning2->SetAttribute("line", "2");
	XMLElement* tuning_step2 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave2 = doc.NewElement("tuning-octave");
	tuning_step2->InsertEndChild(doc.NewText("A"));
	tuning_octave2->InsertEndChild(doc.NewText("2"));					//A2
	staff_tuning2->InsertEndChild(tuning_step2);
	staff_tuning2->InsertEndChild(tuning_octave2);

	XMLElement* staff_tuning3 = doc.NewElement("staff-tuning");
	staff_tuning3->SetAttribute("line", "3");
	XMLElement* tuning_step3 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave3 = doc.NewElement("tuning-octave");
	tuning_step3->InsertEndChild(doc.NewText("D"));
	tuning_octave3->InsertEndChild(doc.NewText("3"));					//D3
	staff_tuning3->InsertEndChild(tuning_step3);
	staff_tuning3->InsertEndChild(tuning_octave3);

	XMLElement* staff_tuning4 = doc.NewElement("staff-tuning");
	staff_tuning4->SetAttribute("line", "4");
	XMLElement* tuning_step4 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave4 = doc.NewElement("tuning-octave");
	tuning_step4->InsertEndChild(doc.NewText("G"));
	tuning_octave4->InsertEndChild(doc.NewText("3"));					//G3
	staff_tuning4->InsertEndChild(tuning_step4);
	staff_tuning4->InsertEndChild(tuning_octave4);

	XMLElement* staff_tuning5 = doc.NewElement("staff-tuning");
	staff_tuning5->SetAttribute("line", "5");
	XMLElement* tuning_step5 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave5 = doc.NewElement("tuning-octave");
	tuning_step5->InsertEndChild(doc.NewText("B"));
	tuning_octave5->InsertEndChild(doc.NewText("3"));					//B3
	staff_tuning5->InsertEndChild(tuning_step5);
	staff_tuning5->InsertEndChild(tuning_octave5);

	XMLElement* staff_tuning6 = doc.NewElement("staff-tuning");
	staff_tuning6->SetAttribute("line", "6");
	XMLElement* tuning_step6 = doc.NewElement("tuning-step");
	XMLElement* tuning_octave6 = doc.NewElement("tuning-octave");
	tuning_step6->InsertEndChild(doc.NewText("E"));
	tuning_octave6->InsertEndChild(doc.NewText("4"));					//E4
	staff_tuning6->InsertEndChild(tuning_step6);
	staff_tuning6->InsertEndChild(tuning_octave6);

	staff_details->InsertEndChild(staff_lines);
	staff_details->InsertEndChild(staff_tuning1);
	staff_details->InsertEndChild(staff_tuning2);
	staff_details->InsertEndChild(staff_tuning3);
	staff_details->InsertEndChild(staff_tuning4);
	staff_details->InsertEndChild(staff_tuning5);
	staff_details->InsertEndChild(staff_tuning6);

	attributes->InsertEndChild(staff_details);
	XMLNode* staff_details2 = staff_details->DeepClone(&doc);
	attributes->InsertEndChild(staff_details2);
	staff_details->SetAttribute("number","1");
	staff_details->NextSiblingElement("staff-details")->SetAttribute("number","2");

	measure->InsertEndChild(attributes);

	int sta = 1;
keepnote:

	vector<note>& srcnotes = toSave.notes;
	size_t n = srcnotes.size();
	XMLElement** notes = new XMLElement*[n];
	XMLElement** pitchs = new XMLElement*[n];
	XMLElement** steps = new XMLElement*[n];
	XMLElement** octaves = new XMLElement*[n];
	XMLElement** durations = new XMLElement*[n];
	XMLElement** voices = new XMLElement*[n];
	XMLElement** types = new XMLElement*[n];
	XMLElement** notations = new XMLElement*[n];
	XMLElement** technicals = new XMLElement*[n];
	XMLElement** frets = new XMLElement*[n];
	XMLElement** strings = new XMLElement*[n];

	
	for (size_t i = 0; i < n; i++) {
		notes[i] = doc.NewElement("note");
		pitchs[i] = doc.NewElement("pitch");
		steps[i] = doc.NewElement("step");
		octaves[i] = doc.NewElement("octave");
		durations[i] = doc.NewElement("duration");
		voices[i] = doc.NewElement("voice");
		types[i] = doc.NewElement("type");
		notations[i] = doc.NewElement("notations");
		strings[i] = doc.NewElement("string");
		frets[i] = doc.NewElement("fret");
		technicals[i] = doc.NewElement("technical");

		if (srcnotes[i].chord) notes[i]->InsertEndChild(doc.NewElement("chord"));
		char pi[3]; 
		bool up;
		switch (srcnotes[i].notation.technical.string)
		{
		case 6:							//E2
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "E2", pi,up);
			break;
		case 5:							//A2
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "A2", pi,up);
			break;
		case 4:							//D3
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "D3", pi,up);
			break;
		case 3:							//G3
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "G3", pi,up);
			break;
		case 2:							//B3
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "B3", pi,up);
			break;
		case 1:							//E4
			pitch(srcnotes[i].notation.technical.string, srcnotes[i].notation.technical.fret, "E4", pi,up);
			break;
		}

		if (srcnotes[i].notation.technical.string == 0) notes[i]->InsertEndChild(doc.NewElement("rest"));
		else {
			char qu[2];
			qu[0] = pi[0];
			qu[1] = 0;
			//if (sta == 1) pi[1] = pi[1] + 1;
			steps[i]->InsertEndChild(doc.NewText(qu));
			octaves[i]->InsertEndChild(doc.NewText(&pi[1]));
			pitchs[i]->InsertEndChild(steps[i]);
			if (up) pitchs[i]->InsertEndChild(doc.NewElement("alter"))->InsertEndChild(doc.NewText("1"));
			pitchs[i]->InsertEndChild(octaves[i]);
			notes[i]->InsertEndChild(pitchs[i]);
		}
		
		//duration: 4:1024

		char a[8] = "";
		_itoa_s(1024 / toSave.time.beats * srcnotes[i].timeValue, a, 10);
		durations[i]->InsertEndChild(doc.NewText(a));
		notes[i]->InsertEndChild(durations[i]);
		notes[i]->InsertEndChild(doc.NewElement("voice"))->InsertEndChild(doc.NewText(sta == 2 ? "5" : "1"));

		switch (srcnotes[i].timeValue)
		{
		case 1:
			types[i]->InsertEndChild(doc.NewText("whole"));
			break;
		case 3:
			types[i]->InsertEndChild(doc.NewText("half"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 2:
			types[i]->InsertEndChild(doc.NewText("half"));
			break;
		case 6:
			types[i]->InsertEndChild(doc.NewText("quarter"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 4:
			types[i]->InsertEndChild(doc.NewText("quarter"));
			break;
		case 12:
			types[i]->InsertEndChild(doc.NewText("eighth"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 8:
			types[i]->InsertEndChild(doc.NewText("eighth"));
			break;
		case 24:
			types[i]->InsertEndChild(doc.NewText("16th"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 16:
			types[i]->InsertEndChild(doc.NewText("16th"));
			break;
		case 48:
			types[i]->InsertEndChild(doc.NewText("32th"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 32:
			types[i]->InsertEndChild(doc.NewText("32th"));
			break;
		case 96:
			types[i]->InsertEndChild(doc.NewText("64th"));
			notes[i]->InsertEndChild(doc.NewElement("dot"));
			break;
		case 64:
			notes[i]->InsertEndChild(doc.NewText("64th"));
			break;
		default:
			raiseErr("timeValue: unexpected value in switch.", 2);
			break;
		}
		notes[i]->InsertEndChild(types[i]);
		notes[i]->InsertEndChild(doc.NewElement("stem"))->InsertEndChild(doc.NewText("up"));
		_itoa_s(sta, a, 10);
		notes[i]->InsertEndChild(doc.NewElement("staff"))->InsertEndChild(doc.NewText(a));
		_itoa_s(srcnotes[i].notation.technical.string, a, 10);
		strings[i]->InsertEndChild(doc.NewText(a));
		_itoa_s(srcnotes[i].notation.technical.fret, a, 10);

		frets[i]->InsertEndChild(doc.NewText(a));

		technicals[i]->InsertEndChild(strings[i]);
		technicals[i]->InsertEndChild(frets[i]);
		notations[i]->InsertEndChild(technicals[i]);
		for (char c : srcnotes[i].notation.notation) {
			switch (c) {
			case 'L': {
				auto tie = doc.NewElement("tie"), tie2 = doc.NewElement("tie");
				tie->SetAttribute("type", "start");
				tie2->SetAttribute("type", "start");
				notes[i]->InsertEndChild(tie);
				notations[i]->InsertEndChild(tie2);
				break;
			}
			case 'l': {
				auto tie = doc.NewElement("tie"), tie2 = doc.NewElement("tie");
				tie->SetAttribute("type", "stop");
				tie2->SetAttribute("type", "stop");
				notations[i]->InsertEndChild(tie);
				notations[i]->InsertEndChild(tie2);
				break;
			}
			case 'H': {
				auto slur = doc.NewElement("slur"), hammerOn = doc.NewElement("hammer-on");
				slur->SetAttribute("type", "start");
				hammerOn->SetAttribute("number", 1);
				hammerOn->SetAttribute("type", "start");
				hammerOn->InsertEndChild(doc.NewText("H"));
				notations[i]->InsertEndChild(slur);
				technicals[i]->InsertEndChild(hammerOn);
				break;
			}
			case 'h': {
				auto slur = doc.NewElement("slur"), hammerOn = doc.NewElement("hammer-on");
				slur->SetAttribute("type", "stop");
				hammerOn->SetAttribute("number", 1);
				hammerOn->SetAttribute("type", "stop");
				notations[i]->InsertEndChild(slur);
				technicals[i]->InsertEndChild(hammerOn);
				break;
			}
			case 'x': {
				auto play = doc.NewElement("play");
				auto mute = doc.NewElement("mute");
				mute->InsertEndChild(doc.NewText("straight"));
				play->InsertEndChild(mute);
				notes[i]->InsertEndChild(play);
			}
			}
		}
		notations[i]->InsertEndChild(doc.NewElement("dynamics"))->InsertEndChild(doc.NewText("mf"));
		notes[i]->InsertEndChild(notations[i]);
		measure->InsertEndChild(notes[i]);
	}
	measure->InsertEndChild(doc.NewElement("backup"))->InsertEndChild(doc.NewElement("duration"))->InsertEndChild(doc.NewText("4096"));
	if (sta < 2) {
		sta++;
		goto keepnote;
	}
	backup.Clear();
	CopyNode(&backup, &doc);
	delete[] notes;
	delete[] pitchs;
	delete[] steps;
	delete[] octaves;
	delete[] durations;
	delete[] voices;
	delete[] types;
	delete[] notations;
	delete[] technicals;
	delete[] frets;
	delete[] strings;

	
}