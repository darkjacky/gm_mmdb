# gm\_mmdb

A GMod module to query [db-ip's IP-to-Country Lite MMDB file](https://db-ip.com/db/download/ip-to-country-lite). This can be used to geofence your server (whitelisting or blacklisting countries) or to enhance the user interface by showing country flags on the scoreboard.

## ⚙️ Performance

Each query takes approximately **0.7 microseconds** on a modern machine, so performance impact should be negligible. (Dedicated functions take less time than the mmdb:LookupField)

Doing a query takes less time than spinning up a thread so this will remain single threaded.

## 🛠️ Compilation Instructions

To compile this yourself:

1. Clone [libmaxminddb](https://github.com/maxmind/libmaxminddb) using Visual Studio 2022.
2. Build a **32-bit or 64-bit Release** configuration.
3. Create a new **Console Application** called `gm_mmdb`.
4. Link the following libraries:

   * `maxminddb.lib`
   * `ws2_32.lib`
5. Clone the [Garry's Mod C++ headers](https://github.com/Facepunch/gmod-module-base) (development branch).
6. Include both the headers from Garry's Mod and the `include` folder from libmaxminddb.

That's it. You should now be able to compile the module.

> **Note**: There is currently no `Close` function implemented, as it is not deemed necessary.

---

## 🧾 Usage

```lua
mmdb:GetIPContinentName( IP )                    -- Same as mmdb:LookupField( IP, "continent", "names", "en" )
mmdb:GetIPContinentCode( IP )                    -- Same as mmdb:LookupField( IP, "continent", "code" )
mmdb:GetIPCountry( IP )                          -- Same as mmdb:LookupField( IP, "country", "iso_code" )
mmdb:GetIPCountryFull( IP )                      -- Same as mmdb:LookupField( IP, "country", "names", "en" )
mmdb:LookupField( IP, "city", "names", "en" )    -- Requires the bigger MMDB; city accuracy usually not very good.
mmdb:GetAllFields( IP )                          -- Significantly slower 4 microseconds per call but this contains all data available for this IP.
```
There is an example file and you can find more examples here [Garry's Mod IP Filter](https://github.com/darkjacky/Garry-s-Mod-IP-Filter)

---

## 📦 Data Format

### ip-to-country-lite

Example data:

```
PrintTable(mmdb:GetAllFields("1.1.1.1"))
["continent"]:
                ["code"]        =       OC
                ["geoname_id"]  =       6255151
                ["names"]:
                                ["de"]  =       Ozeanien
                                ["en"]  =       Oceania
                                ["es"]  =       Oceanía
                                ["fa"]  =       اقیانوسیه
                                ["fr"]  =       Océanie
                                ["ja"]  =       オセアニア
                                ["ko"]  =       오세아니아
                                ["pt-BR"]       =       Oceania
                                ["ru"]  =       Океания
                                ["zh-CN"]       =       大洋洲
["country"]:
                ["geoname_id"]  =       2077456
                ["is_in_european_union"]        =       false
                ["iso_code"]    =       AU
                ["names"]:
                                ["de"]  =       Australien
                                ["en"]  =       Australia
                                ["es"]  =       Australia
                                ["fa"]  =       استرالیا
                                ["fr"]  =       Australie
                                ["ja"]  =       オーストラリア
                                ["ko"]  =       오스트레일리아
                                ["pt-BR"]       =       Austrália
                                ["ru"]  =       Австралия
                                ["zh-CN"]       =       澳大利亚
```

### ip-to-city-lite

Example data:

```
PrintTable(mmdb:GetAllFields("1.1.1.1"))
["city"]:
                ["names"]:
                                ["en"]  =       Sydney
["continent"]:
                ["code"]        =       OC
                ["geoname_id"]  =       6255151
                ["names"]:
                                ["de"]  =       Ozeanien
                                ["en"]  =       Oceania
                                ["es"]  =       Oceanía
                                ["fa"]  =       اقیانوسیه
                                ["fr"]  =       Océanie
                                ["ja"]  =       オセアニア
                                ["ko"]  =       오세아니아
                                ["pt-BR"]       =       Oceania
                                ["ru"]  =       Океания
                                ["zh-CN"]       =       大洋洲
["country"]:
                ["geoname_id"]  =       2077456
                ["is_in_european_union"]        =       false
                ["iso_code"]    =       AU
                ["names"]:
                                ["de"]  =       Australien
                                ["en"]  =       Australia
                                ["es"]  =       Australia
                                ["fa"]  =       استرالیا
                                ["fr"]  =       Australie
                                ["ja"]  =       オーストラリア
                                ["ko"]  =       오스트레일리아
                                ["pt-BR"]       =       Austrália
                                ["ru"]  =       Австралия
                                ["zh-CN"]       =       澳大利亚
["location"]:
                ["latitude"]    =       -33.8688
                ["longitude"]   =       151.209
["subdivisions"]:
                [1]:
                                ["names"]:
                                                ["en"]  =       New South Wales
```

## 🧠 Note on Development

Created with occasional help from GitHub Copilot (not always helpful).
