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

std::string extractIP(const std::string& ipPort) {
    size_t colonPos = ipPort.find(':');
    if (colonPos == std::string::npos) {
        // No colon found, return the whole string or empty
        return ipPort;
    }
    return ipPort.substr(0, colonPos);
}

LUA_FUNCTION(LookupMMDBField) {
    LUA->CheckType(1, mmdbs_ID); // Your MMDB userdata
    LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);

    auto SIP = extractIP(LUA->GetString(2));
	const char* ip = SIP.c_str();

    MMDB_lookup_result_s result;
    int gai_error = 0, mmdb_error = 0;
    result = MMDB_lookup_string(&db->mmdb, ip, &gai_error, &mmdb_error);
    if (!result.found_entry) {
        LUA->PushString("IP not found");
        return 1;
    }
    if (gai_error) {
        LUA->PushString(gai_strerrorA(gai_error));
        return 1;
    }
    if (mmdb_error) {
        LUA->PushString(MMDB_strerror(mmdb_error));
        return 1;
    }

    if (!result.found_entry) {
		LUA->PushString("No entries found");
        return 1;
    }


    // Store full path as std::string to preserve memory
    std::vector<const char*> path_storage;
    int top = LUA->Top();
    for (int i = 3; i <= top; ++i) {
        if (LUA->IsType(i, GarrysMod::Lua::Type::String)) {
            path_storage.emplace_back(LUA->GetString(i));
        }
    }

    path_storage.push_back(nullptr); // NULL terminator

    MMDB_entry_data_s data;
    int status = MMDB_aget_value(&result.entry, &data, path_storage.data());
    if (status != MMDB_SUCCESS || !data.has_data) {
        LUA->PushString("No data found");
        return 1;
    }

    // Push appropriate value to Lua
    switch (data.type) {
    case MMDB_DATA_TYPE_UTF8_STRING:
        LUA->PushString(data.utf8_string, data.data_size);
        break;
    case MMDB_DATA_TYPE_BOOLEAN:
        LUA->PushBool(data.boolean);
        break;
    case MMDB_DATA_TYPE_UINT16:
        LUA->PushNumber(static_cast<double>(data.uint16));
        break;
    case MMDB_DATA_TYPE_UINT32:
        LUA->PushNumber(static_cast<double>(data.uint32));
        break;
    case MMDB_DATA_TYPE_INT32:
        LUA->PushNumber(static_cast<double>(data.int32));
        break;
    case MMDB_DATA_TYPE_DOUBLE:
        LUA->PushNumber(data.double_value);
        break;
    default:
        LUA->PushNil();
        break;
    }

    return 1;
}

LUA_FUNCTION(GetIPContinentName) {
    LUA->CheckType(1, mmdbs_ID);
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

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

LUA_FUNCTION(GetIPContinentcode) {
    LUA->CheckType(1, mmdbs_ID);
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    LuaMMDB* db = LUA->GetUserType<LuaMMDB>(1, mmdbs_ID);
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(&db->mmdb, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushNil();
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "continent", "code", NULL);

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
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

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
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

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
			LUA->PushUserType(db, mmdbs_ID);
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

		LUA->PushCFunction(GetIPContinentcode);
		LUA->SetField(-2, "GetIPContinentcode");

        LUA->PushCFunction(LookupMMDBField);
		LUA->SetField(-2, "LookupField");
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
