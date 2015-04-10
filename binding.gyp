{
  "targets": [
    {
      "target_name": "geoip2c",
      "sources": [ "src/binding.cpp" ],
      'include_dirs': [
          './libmaxminddb/include',
          "<!(node -e \"require('nan')\")"
      ],
      'foo': '<!(cd libmaxminddb && ./bootstrap 2>&1 >/dev/null && ./configure --disable-shared --enable-static >/dev/null && make>/dev/null)',
      'link_settings': {
          'libraries': [
              '-L ../libmaxminddb/src/.libs -lmaxminddb' ]
      }
    }
  ]
}
