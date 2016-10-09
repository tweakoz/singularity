#include <rapidjson/reader.h>
#include <rapidjson/document.h>
#include "krzdata.h"

using namespace rapidjson;

///////////////////////////////////////////////////////////////////////////////

struct VastObjectsDB
{
    void loadJson(const std::string& fname);

    const programData* findProgram(int idx) const;
    const keymap* findKeymap(int kmID) const;

    //

    keymap* parseKeymap( int kmid, const rapidjson::Value& JO );
    AsrData* parseAsr( const rapidjson::Value& JO, const std::string& name );
    LfoData* parseLfo( const rapidjson::Value& JO, const std::string& name );
    FunData* parseFun( const rapidjson::Value& JO, const std::string& name );
    layerData* parseLayer( const rapidjson::Value& JO, programData* pd );
    void parseEnvControl( const rapidjson::Value& JO, EnvCtrlData& ed );
    programData* parseProgram( const rapidjson::Value& JO );
    multisample* parseMultiSample( const rapidjson::Value& JO );
    sample* parseSample( const rapidjson::Value& JO, const multisample* parent );
    
    void parseAlg( const rapidjson::Value& JO, AlgData& algd );
    void parseKmpBlock( const Value& JO, KmpBlockData& kmblk );
    void parsePchBlock( const Value& JO, PchBlockData& pblk );
    void parseFBlock( const Value& JO, FBlockData& fb );
    void parseDspBlock( const Value& JO, DspBlockData& dblk );

    std::map<int,programData*> _programs;
    std::map<int,keymap*> _keymaps;
    std::map<int,multisample*> _multisamples;
};
