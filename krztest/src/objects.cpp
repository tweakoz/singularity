
#include "krzdata.h"
#include "krztest.h"
#include "objectdb.h"

using namespace rapidjson;

///////////////////////////////////////////////////////////////////////////////

keymap* VastObjectsDB::parseKeymap( int kmid, const Value& jsonobj )
{
	auto kmapout = new keymap;

	kmapout->_kmID = kmid;
	kmapout->_name = jsonobj["Keymap"].GetString();
	//printf( "Got Keymap name<%s>\n", kmapout->_name.c_str() );

	const auto& jsonrgns = jsonobj["regions"];
	assert(jsonrgns.IsArray());

	for( SizeType i = 0; i < jsonrgns.Size(); i++) // Uses SizeType instead of size_t
	{	const auto& jsonrgn = jsonrgns[i];

		auto kmr = new kmregion;
		kmr->_lokey = jsonrgn["loKey"].GetInt()+12;
		kmr->_hikey = jsonrgn["hiKey"].GetInt()+12;
		kmr->_lovel = jsonrgn["loVel"].GetInt();
		kmr->_hivel = jsonrgn["hiVel"].GetInt();
		kmr->_tuning = jsonrgn["tuning"].GetInt();
		kmr->_volAdj = jsonrgn["volAdj"].GetFloat();
		kmr->_multsampID = jsonrgn["multiSampleID"].GetInt();
		kmr->_sampID = jsonrgn["subSampleID"].GetInt();

		//printf( "kmr->_msID<%d>\n", kmr->_multsampID );
		//printf( "kmr->_sampID<%d>\n", kmr->_sampID );
		kmr->_sampleName = jsonrgn["sampleName"].GetString();


		kmr->_linGain = decibel_to_linear_amp_ratio(kmr->_volAdj);

		kmapout->_regions.push_back(kmr);
	}
	return kmapout;
}

///////////////////////////////////////////////////////////////////////////////

sample* VastObjectsDB::parseSample( const Value& jsonobj, const multisample* parent )
{
	auto sout = new sample;
	sout->_name = jsonobj["subSampleName"].GetString();
	sout->_subid = jsonobj["subSampleIndex"].GetInt();
	sout->_rootKey = jsonobj["rootKey"].GetInt();
	sout->_highestPitch = jsonobj["highestPitch"].GetInt();


	sout->_blk_start = jsonobj["uStart"].GetInt();
	sout->_blk_alt = jsonobj["uAlt"].GetInt();
	sout->_blk_loop = jsonobj["uLoop"].GetInt();
	sout->_blk_end = jsonobj["uEnd"].GetInt();

	std::string pbmode = jsonobj["playbackMode"].GetString();
	bool isloop = jsonobj["isLooped"].GetBool();

	if( pbmode == "Normal" )
		sout->_loopMode = isloop ? 1 : 0;
	else if( pbmode == "Reverse" )
		sout->_loopMode = isloop ? 2 : 0;
	else
	{
		printf( "pbmode<%s>\n", pbmode.c_str() );
		assert(false);
	}


	float sgain = jsonobj["volAdjust"].GetFloat();
	sout->_linGain = decibel_to_linear_amp_ratio(sgain);

	// natenv
	const auto& jsonnatEnv = jsonobj["natEnv"];

	const auto& jsonneslopes = jsonnatEnv["segSlope (dB/sec)"];
	const auto& jsonnetimes = jsonnatEnv["segTime (sec)"];
	assert(jsonneslopes.IsArray());
	assert(jsonnetimes.IsArray());
	assert(jsonneslopes.Size()==jsonnetimes.Size());
	int numsegs = jsonneslopes.Size();
	sout->_natenv.resize(numsegs);

	for( SizeType i = 0; i<numsegs; i++) // Uses SizeType instead of size_t
	{	
		auto& dest = sout->_natenv[i];
		dest._slope = jsonneslopes[i].GetFloat();
		dest._time = jsonnetimes[i].GetFloat();
	}


	//



	int parid = parent->_objid;

	auto fname = formatString("samples/%03d_%s_%d.aiff",parid,parent->_name.c_str(), sout->_subid );

	//printf( "fname<%s>\n", fname.c_str() );

	//sout->load(fname);

	sout->_sampleRate = jsonobj["sampleRate"].GetFloat();

	return sout;
}

///////////////////////////////////////////////////////////////////////////////

multisample* VastObjectsDB::parseMultiSample( const Value& jsonobj )
{
	auto msout = new multisample;
	msout->_name = jsonobj["MultiSample"].GetString();
	msout->_objid = jsonobj["objectID"].GetInt();
	//printf( "Got MultiSample name<%s>\n", msout->_name.c_str() );
	const auto& jsonsamps = jsonobj["samples"];
	assert(jsonsamps.IsArray());

	for( SizeType i = 0; i < jsonsamps.Size(); i++) // Uses SizeType instead of size_t
	{	
		auto s = parseSample(jsonsamps[i],msout);
		msout->_samples[s->_subid] = s;
	}
	return msout;
}

///////////////////////////////////////////////////////////////////////////////

AsrData* VastObjectsDB::parseAsr( const rapidjson::Value& jo, const std::string& name )
{
	auto aout = new AsrData;
	aout->_trigger = jo["trigger"].GetString();
	aout->_mode = jo["mode"].GetString();
	aout->_delay = jo["delay"].GetFloat();
	aout->_attack = jo["attack"].GetFloat();
	aout->_release = jo["release"].GetFloat();
	aout->_name = name;

	return aout;
}

///////////////////////////////////////////////////////////////////////////////

LfoData* VastObjectsDB::parseLfo( const rapidjson::Value& jo, const std::string& name )
{
	auto lout = new LfoData;
	lout->_initialPhase = jo["phase"].GetFloat();
	lout->_shape = jo["shape"].GetString();
	lout->_controller = jo["rateCtl"].GetString();
	lout->_minRate = jo["minRate(hz)"].GetFloat();
	lout->_maxRate = jo["maxRate(hz)"].GetFloat();
	lout->_name = name;

	return lout;
}

///////////////////////////////////////////////////////////////////////////////

FunData* VastObjectsDB::parseFun( const rapidjson::Value& jo, const std::string& name )
{
	auto out = new FunData;
	out->_a = jo["a"].GetString();
	out->_b = jo["b"].GetString();
	out->_op = jo["op"].GetString();
	out->_name = name;
	return out;
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseFBlock( const Value& fseg, FBlockData& fblk )
{
	//////////////////////////////////
	// default evaluator
	//////////////////////////////////

	fblk._mods._evaluator = [](float coarse, float s1, float s2) -> float
	{
		return coarse+s1+s2;
	};

	//////////////////////////////////

	if( fseg.HasMember("KeyTrack") )
		fblk._keyTrack = fseg["KeyTrack"]["Value"].GetFloat();
	if( fseg.HasMember("VelTrack") )
		fblk._velTrack = fseg["VelTrack"]["Value"].GetFloat();
	if( fseg.HasMember("Coarse") )
	{	auto& c = fseg["Coarse"];
		assert(c.HasMember("Value"));
		fblk._units = c["Unit"].GetString();
		auto& v = c["Value"];
		switch( v.GetType() )
		{	case kNumberType:
				fblk._coarse = v.GetFloat();
				break;
			case kStringType:
			{	auto toks = SplitString(v.GetString(), ' ');
				auto snote = toks[0];
				auto soct = toks[1];
				int inote = 0;
				int ioct = atoi(soct.c_str());
				if(snote=="C") inote=0;
				if(snote=="C#") inote=1;
				if(snote=="D") inote=2;
				if(snote=="D#") inote=3;
				if(snote=="E") inote=4;
				if(snote=="F") inote=5;
				if(snote=="F#") inote=6;
				if(snote=="G") inote=7;
				if(snote=="G#") inote=8;
				if(snote=="A") inote=9;
				if(snote=="A#") inote=10;
				if(snote=="B") inote=11;
				int midinote = ioct*12 + inote;
				float frq = midi_note_to_frequency(midinote);
				fblk._coarse = frq;
				printf( "v.GetString() %s\n", v.GetString() );
				printf( "inote<%d> ioct<%d> midinote<%d> frq<%f>\n", inote, ioct, midinote, frq );
				//assert(false);
				assert(c["Unit"]=="nt");

				// note/cent evaluator
				fblk._mods._evaluator = [](float coarseHz, float s1Cents, float s2Cents) -> float
				{
					float totcents = s1Cents+s2Cents;
					float ratio = cents_to_linear_freq_ratio(totcents);
					//printf( "fbase<%f> totc<%f> ratio<")
					return coarseHz*ratio;
				};

				break;
			}
			default:
				assert(false);
				break;
		}
	}
	if( fseg.HasMember("Src1") )
	{	auto& s1 = fseg["Src1"];
		fblk._mods._src1 = s1["Source"].GetString();
		if( s1.HasMember("Depth") )
		{	auto& d = s1["Depth"];
			fblk._mods._src1Depth = d["Value"].GetFloat();
		}
	}
	if( fseg.HasMember("Src2") )
	{	auto& s = fseg["Src2"];
		fblk._mods._src2 = s["Source"].GetString();
		if( s.HasMember("DepthControl") )
			fblk._mods._src2DepthCtrl = s["DepthControl"].GetString();
		if( s.HasMember("MinDepth") )
			fblk._mods._src2MinDepth = s["MinDepth"]["Value"].GetFloat();
		if( s.HasMember("MaxDepth") )
			fblk._mods._src2MaxDepth = s["MaxDepth"]["Value"].GetFloat();
	}



	//////////////////////////////////


	//////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parsePchBlock( const Value& pseg, PchBlockData& pblk )
{
	parseFBlock(pseg,pblk);
	if( pseg.HasMember("Fine") and pseg["Fine"].IsNumber() )
		pblk._fine = pseg["Fine"].GetFloat();
	if( pseg.HasMember("FineHZ") and pseg["FineHZ"].IsNumber() )
		pblk._fineHZ = pseg["FineHZ"].GetFloat();

}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseKmpBlock( const Value& kmseg, KmpBlockData& kmblk )
{
	kmblk._keyTrack = kmseg["KeyTrack(ct)"].GetInt();
	kmblk._transpose = kmseg["transposeTS(st)"].GetInt();
	kmblk._timbreShift = kmseg["timbreshift"].GetInt();
	kmblk._pbMode = kmseg["pbmode"].GetString();
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseDspBlock( const Value& dseg, DspBlockData& dblk )
{
	if( dseg.HasMember("BLOCK_ALG ") )
	{
		dblk._dspBlock = dseg["BLOCK_ALG "].GetString();
	}
	if( dseg.HasMember("PARAM_SCHEME") )
		dblk._paramScheme = dseg["PARAM_SCHEME"].GetString();
	if( dseg.HasMember("Pad") )
	{	dblk._pad = decibel_to_linear_amp_ratio(dseg["Pad"].GetFloat());
		//assert(false);
	}
	parseFBlock(dseg,dblk);
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseAmpBlock( const Value& aseg, AmpBlockData& ablk )
{
	parseFBlock(aseg,ablk);
	ablk._pad = aseg["Pad"].GetFloat();
}

///////////////////////////////////////////////////////////////////////////////

layerData* VastObjectsDB::parseLayer( const Value& jsonobj, programData* pd )
{
	const auto& name = pd->_name;
	printf( "Got Prgram<%s> layer..\n", name.c_str() );
	const auto& calvinSeg = jsonobj["CALVIN"];
	const auto& keymapSeg = calvinSeg["KEYMAP"];
	const auto& pitchSeg = calvinSeg["PITCH"];
	const auto& miscSeg = jsonobj["misc"];

	int kmid = keymapSeg["km1"].GetInt();
	//printf( "find KMID<%d>\n", kmid );
	auto it = _keymaps.find(kmid);
	assert(it!=_keymaps.end());

	auto rval = pd->newLayer();
	auto km = it->second;
	rval->_keymap = km;

	parseKmpBlock(keymapSeg,rval->_kmpBlock);
	parsePchBlock(pitchSeg,rval->_pchBlock);

	rval->_loKey = jsonobj["loKey"].GetInt();
	rval->_hiKey = jsonobj["hiKey"].GetInt();
	rval->_loVel = jsonobj["loVel"].GetInt()*18;
	rval->_hiVel = 1+(jsonobj["hiVel"].GetInt()*18);
	if( rval->_loVel == 0 && rval->_hiVel == 0 )
		rval->_hiVel = 127;

	rval->_ignRels = miscSeg["ignRels"].GetBool();
	rval->_atk1Hold = miscSeg["atkHold"].GetBool();
	rval->_atk3Hold = miscSeg["susHold"].GetBool();

	parseAlg(jsonobj,rval->_algData);

	//////////////////////////////////////////////////////

	const auto& envcSeg = jsonobj["ENVCTRL"];
	parseEnvControl(envcSeg,rval->_envCtrlData );

	auto parseEnv = [&](const Value& envobj, const std::string& name) -> RateLevelEnvData*
	{
		auto rout = new RateLevelEnvData;
		RateLevelEnvData&destenv = *rout;
		rout->_name = name;
		//////////////////////////////////////////
		const auto& jsonrates = envobj["rates"];
		assert(jsonrates.IsArray());
		int inumrates = jsonrates.Size();
		assert(inumrates==7);
		std::vector<float> rates;
		for( SizeType i = 0; i < inumrates; i++) // Uses SizeType instead of size_t
			rates.push_back(jsonrates[i].GetFloat());
		//////////////////////////////////////////
		const auto& jsonlevels = envobj["levels"];
		assert(jsonlevels.IsArray());
		int inumlevels = jsonlevels.Size();
		assert(inumlevels==7);
		std::vector<float> levels;
		for( SizeType i = 0; i < inumlevels; i++) // Uses SizeType instead of size_t
			levels.push_back(jsonlevels[i].GetFloat());
		//////////////////////////////////////////
		for( int i=0; i<7; i++ )
		{
			EnvPoint ep;
			ep._rate = rates[i];
			ep._level = levels[i];
			destenv._segments.push_back(ep);
		}
		//////////////////////////////////////////
		return rout;
	};
	if( jsonobj.HasMember("AMPENV") )
	{	const auto& seg = jsonobj["AMPENV"];
		if( seg.IsObject() )
			rval->_userAmpEnv = parseEnv(seg,"AMPENV");
	}
	if( jsonobj.HasMember("ENV2") )
	{	const auto& seg = jsonobj["ENV2"];
		if( seg.IsObject() )
			rval->_env2 = parseEnv(seg,"ENV2");
	}
	if( jsonobj.HasMember("ENV3") )
	{	const auto& seg = jsonobj["ENV3"];
		if( seg.IsObject() )
			rval->_env3 = parseEnv(seg,"ENV3");
	}
	//////////////////////////////////////////////////////
	if( jsonobj.HasMember("ASR1") )
	{	const auto& seg = jsonobj["ASR1"];
		if( seg.IsObject() )
		{
			rval->_asr1 = parseAsr(seg,"ASR1");
		}
	}
	if( jsonobj.HasMember("ASR2") )
	{	const auto& seg = jsonobj["ASR2"];
		if( seg.IsObject() )
		{
			rval->_asr2 = parseAsr(seg,"ASR2");
		}
	}
	//////////////////////////////////////////////////////
	if( jsonobj.HasMember("LFO1") )
	{	
		const auto& lfo1seg = jsonobj["LFO1"];
		if( lfo1seg.IsObject() )
			rval->_lfo1 = parseLfo(lfo1seg, "LFO1");
	}
	if( jsonobj.HasMember("LFO2") )
	{	const auto& lfo2seg = jsonobj["LFO2"];
		if( lfo2seg.IsObject() )
			rval->_lfo2 = parseLfo(lfo2seg, "LFO2");
	}
	//////////////////////////////////////////////////////
	if( jsonobj.HasMember("FUN1") )
	{	const auto& seg = jsonobj["FUN1"];
		if( seg.IsObject() )
			rval->_fun1 = parseFun(seg,"FUN1");
	}
	if( jsonobj.HasMember("FUN2") )
	{	const auto& seg = jsonobj["FUN2"];
		if( seg.IsObject() )
			rval->_fun2 = parseFun(seg,"FUN2");
	}
	if( jsonobj.HasMember("FUN3") )
	{	const auto& seg = jsonobj["FUN3"];
		if( seg.IsObject() )
			rval->_fun3 = parseFun(seg,"FUN3");
	}
	if( jsonobj.HasMember("FUN4") )
	{	const auto& seg = jsonobj["FUN4"];
		if( seg.IsObject() )
			rval->_fun4 = parseFun(seg,"FUN4");
	}
	//////////////////////////////////////////////////////
	rval->_f1Block._blockIndex = 0;
	rval->_f2Block._blockIndex = 1;
	rval->_f3Block._blockIndex = 2;
	//rval->_ampBlock._blockIndex = 3;
	if( jsonobj.HasMember("F1") )
		parseDspBlock(jsonobj["F1"],rval->_f1Block);
	if( jsonobj.HasMember("F2") )
		parseDspBlock(jsonobj["F2"],rval->_f2Block);
	if( jsonobj.HasMember("F3") )
		parseDspBlock(jsonobj["F3"],rval->_f3Block);
	if( jsonobj.HasMember("F4AMP") )
		parseAmpBlock(jsonobj["F4AMP"],rval->_ampBlock);
	//////////////////////////////////////////////////////

	//printf( "got keymapID<%d:%p:%s>\n", kmid, km, km->_name.c_str() );
	return rval;

}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseAlg( const rapidjson::Value& JO, AlgData& algd )
{
	const auto& calvin = JO["CALVIN"];
	algd._algID = calvin["ALG"].GetInt();
	algd._name = formatString("ALG%d", algd._algID);
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::parseEnvControl( const rapidjson::Value& seg, EnvCtrlData& ed )
{
	auto aenvmode = seg["ampenv_mode"].GetString();
	ed._useNatEnv = (0==strcmp(aenvmode,"Natural"));
	ed._atkAdjust = seg["AtkAdjust"].GetFloat();
	ed._decAdjust = seg["DecAdjust"].GetFloat();
	ed._relAdjust = seg["RelAdjust"].GetFloat();

	ed._decKeyTrack = seg["DecKeyTrack"].GetFloat();
	ed._relKeyTrack = seg["RelKeyTrack"].GetFloat();
}

///////////////////////////////////////////////////////////////////////////////

programData* VastObjectsDB::parseProgram( const Value& jsonobj )
{
	auto pdata = new programData;
	const auto& name = jsonobj["Program"].GetString();
	//printf( "Got Prgram name<%s>\n", name );
	pdata->_name = name;

	const auto& jsonlays = jsonobj["LAYERS"];
	assert(jsonlays.IsArray());

	for( SizeType i = 0; i < jsonlays.Size(); i++) // Uses SizeType instead of size_t
	{	
		auto l = parseLayer(jsonlays[i],pdata);
	}

	return pdata;
}

///////////////////////////////////////////////////////////////////////////////

void VastObjectsDB::loadJson( const std::string& fname )
{
	FILE* fin = fopen(fname.c_str(),"rt");
	fseek(fin,0,SEEK_END);
	int size = ftell(fin);
	printf( "fname<%s> length<%d>\n", fname.c_str(), size );
	fseek(fin,0,SEEK_SET);
	auto jsondata = (char*) malloc(size+1);
	fread(jsondata,size,1,fin);
	jsondata[size] = 0;
	fclose(fin);

	Document document;
	document.Parse(jsondata);
	free((void*)jsondata);

	auto objmap = this;

	assert(document.IsObject());
	assert(document.HasMember("KRZ"));

	const auto& root = document["KRZ"];
	const auto& objects = root["objects"];
	assert(objects.IsArray());

	for( SizeType i = 0; i < objects.Size(); i++) // Uses SizeType instead of size_t
	{
		const auto& obj = objects[i];
		int objid = obj["objectID"].GetInt();
		if( obj.HasMember("Keymap") )
		{
			auto km = parseKeymap(objid,obj);
			objmap->_keymaps[objid] = km;
		}
		else if( obj.HasMember("MultiSample") )
		{
			auto ms = parseMultiSample(obj);
			objmap->_multisamples[objid] = ms;
		}
		else if( obj.HasMember("Program") )
		{
			auto p = this->parseProgram(obj);
			objmap->_programs[objid] = p;
		}
		else
		{
			assert(false);
		}
	}

	for( auto& item : _keymaps )
	{
		auto km = item.second;
		for( auto kr : km->_regions )
		{
			int msid = kr->_multsampID;
			auto msit = _multisamples.find(msid);
			assert(msit!=_multisamples.end());
			kr->_multiSample = msit->second;
			int sid = kr->_sampID;
			auto& samplemap = kr->_multiSample->_samples;
			auto sit = samplemap.find(sid);
			if( sit == samplemap.end() )
			{
				//printf( "sample<%d> not found in multisample<%d>\n", sid, msid );
			}
			else
			{
				auto s = sit->second;
				kr->_sample = s;
				//printf( "found sample<%d:%s> in multisample<%d>\n", sid, s->_name.c_str(), msid );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

const programData* VastObjectsDB::findProgram(int progID) const
{
	programData* pd = nullptr;
	auto it = _programs.find(progID);
	assert(it!=_programs.end());
	pd = it->second;
	return pd;
}

///////////////////////////////////////////////////////////////////////////////

const keymap* VastObjectsDB::findKeymap(int kmID) const
{
	keymap* kd = nullptr;
	auto it = _keymaps.find(kmID);
	if(it!=_keymaps.end())
		kd = it->second;
	return kd;
}
