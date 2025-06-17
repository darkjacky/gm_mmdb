require( "mmdb" )

-- The MMDB.Open opens from the base directory which is the folder where your srcds.exe (or gmod.exe) is located.
local mmdb = MMDB.Open( "garrysmod/mmdb/dbip-country-lite-2025-06.mmdb" )

print( mmdb:GetIPCountry( "1.1.1.1" ), mmdb:GetIPCountryFull( "1.1.1.1" ), mmdb:GetIPContinentName("1.1.1.1") ) -- Cloudflare DNS IP
-- Outputs: AU      Australia       Oceania
