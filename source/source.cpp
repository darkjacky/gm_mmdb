#include <GarrysMod/Lua/Interface.h>
#include <maxminddb.h>
#include <string>
#include <vector>

class LuaMMDB {
public:
    MMDB_s mmdb;
	std::string db_path;
};

std::vector<LuaMMDB *> mmdbs;
int mmdbs_ID = 0;

LUA_FUNCTION(GetIPContinentName) {
    LUA->CheckType(1, mmdbs_ID);
    const char* ip = LUA->CheckString(2);

    LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(&db->mmdb, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushNil();
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "continent", "names", "en", NULL);

    if (status == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
        LUA->PushString(entry_data.utf8_string, entry_data.data_size);
    }
    else {
        LUA->PushNil();
    }

    return 1;
}

LUA_FUNCTION(GetIPCountry) {
    LUA->CheckType(1, mmdbs_ID);
    const char* ip = LUA->CheckString(2);

    int gai_error, mmdb_error;
    LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);
    MMDB_lookup_result_s result = MMDB_lookup_string(&db->mmdb, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushString("Unknown");
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);
    if (status == MMDB_SUCCESS && entry_data.has_data) {
        LUA->PushString(std::string(entry_data.utf8_string, entry_data.data_size).c_str());
    }
    else {
        LUA->PushString("Unknown");
    }

    return 1;
}

LUA_FUNCTION(GetIPCountryFull) {
    LUA->CheckType(1, mmdbs_ID);
    const char* ip = LUA->CheckString(2);

    int gai_error, mmdb_error;
    LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);
    MMDB_lookup_result_s result = MMDB_lookup_string(&db->mmdb, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushString("Unknown");
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "country", "names", "en", NULL);
    if (status == MMDB_SUCCESS && entry_data.has_data) {
        LUA->PushString(std::string(entry_data.utf8_string, entry_data.data_size).c_str());
    }
    else {
        LUA->PushString("Unknown");
    }

    return 1;
}

LUA_FUNCTION(ToStringMMDB) {
	LUA->CheckType(1, mmdbs_ID);
	LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);
	if (db) {
		LUA->PushString(("LuaMMDB: " + db->db_path).c_str());
	}
	else {
		LUA->PushString("Invalid LuaMMDB");
	}
	return 1;
}

LUA_FUNCTION(OpenMMDB) {
	const char* db_path = LUA->CheckString(1);

	for (auto& db : mmdbs) {
		if (db->db_path == db_path) {
			LUA->PushUserType(&db, mmdbs_ID);
			return 1;
		}
	}

    LuaMMDB * newdb = new LuaMMDB;

	if (MMDB_open(db_path, MMDB_MODE_MMAP, &newdb->mmdb) != MMDB_SUCCESS) {
		delete newdb;
		LUA->ThrowError("Failed to open MMDB file");
		return 0;
	}

	newdb->db_path = db_path;
    mmdbs.push_back( newdb );

	LUA->PushUserType(newdb, mmdbs_ID);

	return 1;
}

GMOD_MODULE_OPEN() {

    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA->CreateTable();
		LUA->PushCFunction(OpenMMDB);
		LUA->SetField(-2, "Open");
	LUA->SetField(-2, "MMDB");
    LUA->Pop();


    mmdbs_ID = LUA->CreateMetaTable("LuaMMDB");
    {
        LUA->Push(-1);                      // push the metatable
        LUA->SetField(-2, "__index");       // metatable.__index = metatable

        LUA->PushCFunction(ToStringMMDB);
        LUA->SetField(-2, "__tostring");

		LUA->PushCFunction(GetIPCountry);
		LUA->SetField(-2, "GetIPCountry");

		LUA->PushCFunction(GetIPCountryFull);
		LUA->SetField(-2, "GetIPCountryFull");

		LUA->PushCFunction(GetIPContinentName);
		LUA->SetField(-2, "GetIPContinentName");
    }
    LUA->Pop(); // Pop metatable off stack


    return 0;
}

GMOD_MODULE_CLOSE() {
    for (auto& db : mmdbs) {
        MMDB_close(&db->mmdb);
        delete db;
    }
    return 0;
}
