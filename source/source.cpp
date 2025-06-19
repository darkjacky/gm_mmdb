#include <GarrysMod/Lua/Interface.h>
#include <maxminddb.h>
#include <string>
#include <vector>

int mmdbs_ID = 0;

// Pushes the entry data list to Lua as a table. This function recursively processes the entry data list and pushes it to the Lua stack.
static void PushEntryDataListToLua(GarrysMod::Lua::ILuaBase* LUA, MMDB_entry_data_list_s** entry_data_list) {
    MMDB_entry_data_list_s* current = *entry_data_list;

    if (!current) return;

    switch (current->entry_data.type) {
    case MMDB_DATA_TYPE_MAP: {
        uint32_t size = current->entry_data.data_size;
        LUA->CreateTable();
        current = current->next;

        for (uint32_t i = 0; i < size; i++) {
            if (!current || current->entry_data.type != MMDB_DATA_TYPE_UTF8_STRING) break;

            // Key
            LUA->PushString(current->entry_data.utf8_string, current->entry_data.data_size);
            current = current->next;

            // Value (recurse)
            PushEntryDataListToLua(LUA, &current);

            // table[key] = value
            LUA->RawSet(-3);
        }

        *entry_data_list = current;
        break;
    }

    case MMDB_DATA_TYPE_ARRAY: {
        uint32_t size = current->entry_data.data_size;
        LUA->CreateTable();
        current = current->next;

        for (uint32_t i = 0; i < size; i++) {
            // Value
            LUA->PushNumber(i + 1);
            PushEntryDataListToLua(LUA, &current);
            LUA->RawSet(-3);
        }

        *entry_data_list = current;
        break;
    }

    case MMDB_DATA_TYPE_UTF8_STRING:
        LUA->PushString(current->entry_data.utf8_string, current->entry_data.data_size);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_BOOLEAN:
        LUA->PushBool(current->entry_data.boolean);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_DOUBLE:
        LUA->PushNumber(current->entry_data.double_value);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_FLOAT:
        LUA->PushNumber(current->entry_data.float_value);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_UINT16:
        LUA->PushNumber(current->entry_data.uint16);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_UINT32:
        LUA->PushNumber(current->entry_data.uint32);
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_UINT64:
        LUA->PushNumber(static_cast<double>(current->entry_data.uint64));
        current = current->next;
        *entry_data_list = current;
        break;

    case MMDB_DATA_TYPE_INT32:
        LUA->PushNumber(current->entry_data.int32);
        current = current->next;
        *entry_data_list = current;
        break;

    default:
        LUA->PushNil();
        current = current->next;
        *entry_data_list = current;
        break;
    }
}

// Takes any string splits it into before comma and after comma then returns the part before the comma. Quick and easy way to split an IPv4 address.
static std::string extractIP(const std::string& ipPort) {
    size_t colonPos = ipPort.find(':');
    if (colonPos == std::string::npos) {
        // No colon found, return the whole string or empty
        return ipPort;
    }
    return ipPort.substr(0, colonPos);
}

/*
Example returns:
["continent"]:
                ["code"]        =       NA
                ["geoname_id"]  =       6255149
                ["names"]:
                                ["en"]  =       North America
["country"]:
                ["geoname_id"]  =       6252001
                ["is_in_european_union"]        =       false
                ["iso_code"]    =       US
                ["names"]:
                                ["en"]  =       United States
*/
static LUA_FUNCTION(GetAllFields) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushNil();
        return 1;
    }

    MMDB_entry_data_list_s* list = nullptr;
    if (MMDB_get_entry_data_list(&result.entry, &list) != MMDB_SUCCESS || !list) {
        LUA->PushNil();
        return 1;
    }

    PushEntryDataListToLua(LUA, &list);
    MMDB_free_entry_data_list(list);
    return 1;
}

// Looks up a specific field in the MMDB database and returns its value.
static LUA_FUNCTION(LookupMMDBField) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
	const char* ip = SIP.c_str();

    MMDB_lookup_result_s result;
    int gai_error = 0, mmdb_error = 0;
    result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);
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

// Example returns: North America
static LUA_FUNCTION(GetIPContinentName) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

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

// Example returns: NA
static LUA_FUNCTION(GetIPContinentcode) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

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

// Example returns: 37.4223, -122.085
static LUA_FUNCTION(GetIPCoordinates) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushNil();
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "location", "latitude", NULL);

    if (status == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_DOUBLE) {
        LUA->PushNumber(entry_data.double_value);

        int status = MMDB_get_value(&result.entry, &entry_data, "location", "longitude", NULL);
        if (status == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_DOUBLE) {
            LUA->PushNumber(entry_data.double_value);
            return 2;
        }
        LUA->PushNumber(0.0); // falback
        return 2;
    }

    LUA->PushNumber(0.0);
    LUA->PushNumber(0.0);

    return 2;
}

// Example returns: Mountain View
static LUA_FUNCTION(GetIPCityName) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");

    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

    if (!result.found_entry) {
        LUA->PushNil();
        return 1;
    }

    MMDB_entry_data_s entry_data;
    int status = MMDB_get_value(&result.entry, &entry_data, "city", "names", "en", NULL);

    if (status == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
        LUA->PushString(entry_data.utf8_string, entry_data.data_size);
    }
    else {
        LUA->PushNil();
    }

    return 1;
}

// Example returns: US
static LUA_FUNCTION(GetIPCountry) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

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

// Example returns: United States
static LUA_FUNCTION(GetIPCountryFull) {
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
    if (db == nullptr || !db->filename) LUA->ThrowError("Invalid MMDB handle or already closed");
    auto SIP = extractIP(LUA->GetString(2));
    const char* ip = SIP.c_str();

    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(db, ip, &gai_error, &mmdb_error);

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

// Converts the MMDB_s object to a string representation for debugging purposes.
static LUA_FUNCTION(ToStringMMDB) {
	LUA->CheckType(1, mmdbs_ID);
    MMDB_s* db = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
	if (db) {
		std::string db_path = db->filename ? db->filename : "Invalid LuaMMDB";
		LUA->PushString(("LuaMMDB: " + db_path).c_str());
	} else {
		LUA->PushString("Invalid LuaMMDB");
	}
	return 1;
}

std::vector<MMDB_s> mmdbs;

// Opens a new MMDB file pushes it to the std::vector and returns a LuaMMDB object.
static LUA_FUNCTION(OpenMMDB) {
    const char* db_path = LUA->CheckString(1);

    // Check if already opened
    for (auto& db : mmdbs) {
        if (strcmp(db.filename, db_path) == 0) {
            LUA->PushUserType(&db, mmdbs_ID);
            return 1;
        }
    }

    // Allocate and open new MMDB
    MMDB_s newdb;
    if (MMDB_open(db_path, MMDB_MODE_MMAP, &newdb) != MMDB_SUCCESS) {
        LUA->ThrowError("Failed to open MMDB file");
        return 0;
    }

    // Store MMDB_s in std::vector
    mmdbs.emplace_back(std::move(newdb));
    MMDB_s* stored_db = &mmdbs.back();  // Get pointer to stored object

    LUA->PushUserType(stored_db, mmdbs_ID);
    return 1;
}

// Closes the MMDB and removes it from the std::vector.
static LUA_FUNCTION(CloseMMDB) {
    MMDB_s* dbptr = LUA->GetUserType<MMDB_s>(1, mmdbs_ID);
	if (dbptr == nullptr || !dbptr->filename) {
		LUA->ThrowError("Invalid MMDB handle or already closed");
		return 0;
	}

    auto it = std::find_if(mmdbs.begin(), mmdbs.end(), [&](MMDB_s& db) {
        return &db == dbptr;
    });

    if (it == mmdbs.end()) {
        LUA->ThrowError("Invalid MMDB handle or already closed");
        return 0;
    }

    MMDB_close(&(*it));
    mmdbs.erase(it);

    return 0;
}

GMOD_MODULE_OPEN() {
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA->CreateTable();
    {
        LUA->PushCFunction(OpenMMDB);
        LUA->SetField(-2, "Open");

        LUA->PushCFunction(CloseMMDB);
        LUA->SetField(-2, "Close");
    }
	LUA->SetField(-2, "MMDB");
    LUA->Pop();

    mmdbs_ID = LUA->CreateMetaTable("LuaMMDB");
    {
        LUA->Push(-1);                      // push the metatable
        LUA->SetField(-2, "__index");       // metatable.__index = metatable

        LUA->PushCFunction(ToStringMMDB);
        LUA->SetField(-2, "__tostring");

        LUA->PushCFunction(CloseMMDB);
        LUA->SetField(-2, "__gc");

        LUA->PushCFunction(CloseMMDB);
        LUA->SetField(-2, "Close");

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

        LUA->PushCFunction(GetAllFields);
		LUA->SetField(-2, "GetAllFields");

		LUA->PushCFunction(GetIPCoordinates);
		LUA->SetField(-2, "GetIPCoordinates");

		LUA->PushCFunction(GetIPCityName);
		LUA->SetField(-2, "GetIPCityName");
    }
    LUA->Pop(); // Pop metatable off stack

    return 0;
}

GMOD_MODULE_CLOSE() {
	for (auto& db : mmdbs) {
		MMDB_close(&db);
	}
	mmdbs.clear();
    return 0;
}
