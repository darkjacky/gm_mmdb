# gm_mmdb

This is made to read the MMDB file from https://db-ip.com/db/download/ip-to-country-lite

This would allow you to geofence your server from users in certain countries (whitelist and blacklist). Or use the IP to display a flag in the scoreboard.
Each query takes about 0.0000007 seconds on my computer so it shouldn't give any issues with performance.
I made this with the help of copilot. (As far as it was helpful which it really wasn't some times.)

To compile this yourself you will need to clone the libmaxminddb github with visual studio (2022) then create a 32 bit or 64 bit release and hit build all.
After that create a new console application called gm_mmdb and link maxminddb.lib and ws2_32.lib. Make sure to git clone the garrys mod c++ headers in the development branch and include that and the include folder of libmaxminddb.
This should be enough to compile it yourself.

Currently there is no Close function as I don't deem it a requirement.

mmdb:LookupField allows you to lookup any arbitrary field from the mmdb.
mmdb:LookupField( IP, "continent", "names", "en" ) is the same as mmdb:GetIPContinentName( IP )
mmdb:LookupField( IP, "continent", "code" ) is the same as mmdb:GetIPContinentcode( IP )
mmdb:LookupField( IP, "country", "iso_code" ) is the same as mmdb:GetIPCountry( IP )
mmdb:LookupField( IP, "country", "names", "en" ) is the same as mmdb:GetIPCountryFull( IP )
If you download the bigger database you can do mmdb:LookupField( IP, "city", "names", "en" ) to look up the city. The city often is not acurate however so it is probably best to stick with the smaller mmdb.

The ip-to-country-lite has the following data:
  continent:
    code: OC
    geoname_id: 6255151
    names:
      de: Ozeanien
      en: Oceania
      es: Oceanía
      fa: اقیانوسیه
      fr: Océanie
      ja: オセアニア
      ko: 오세아니아
      pt-BR: Oceania
      ru: Океания
      zh-CN: 大洋洲
  country:
    geoname_id: 2077456
    is_in_european_union: false
    iso_code: AU
    names:
      de: Australien
      en: Australia
      es: Australia
      fa: استرالیا
      fr: Australie
      ja: オーストラリア
      ko: 오스트레일리아
      pt-BR: Austrália
      ru: Австралия
      zh-CN: 澳大利亚


The ip-to-city-lite has the following data:
  city:
    names:
      en: Sydney
  continent:
    code: OC
    geoname_id: 6255151
    names:
      de: Ozeanien
      en: Oceania
      es: Oceanía
      fa: اقیانوسیه
      fr: Océanie
      ja: オセアニア
      ko: 오세아니아
      pt-BR: Oceania
      ru: Океания
      zh-CN: 大洋洲
  country:
    geoname_id: 2077456
    is_in_european_union: false
    iso_code: AU
    names:
      de: Australien
      en: Australia
      es: Australia
      fa: استرالیا
      fr: Australie
      ja: オーストラリア
      ko: 오스트레일리아
      pt-BR: Austrália
      ru: Австралия
      zh-CN: 澳大利亚
  location:
    latitude: -33.8688
    longitude: 151.209
  subdivisions:
  - names:
      en: New South Wales

You can find this with https://github.com/maxmind/mmdbinspect
