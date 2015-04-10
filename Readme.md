## Requirements

   This package is currently designed to build and run only on *nix
(tested only on Linux). It should be technically possible
to adapt it for Windows as well.
The package includes and builds the libmaxminddb library from source.
To to so, it needs the autotools toolchain. More specificly - autoconf,
libtool, automake. Also, it needs node-gyp installed:
    npm install -g node-gyp

## Usage

    var geo = require('geoip2c');
    geo.load('path/to/database/file',['country','city','isp','netspeed']);
    var info = geo.lookupIp('1.2.3.4');
    geo.unload()  
to free memory.

##Notes on libmaxminddb
Libmaxminddb is built by this package as a static lib, and linked into
the node module shared object (.node file). This means that you can't
update the version used without rebuilding the module.

The version of libmaxminddb, included with this package has a
slightly modified build system. It doesn't build the 't' subdirectory,
containing tests, because it has a dependency on libtap. Also, it adds
the -fPIC flag (in common.mk) because the generated code, even though
of a static lib, is later linked into a shared object, so the code must
be position-independent.
	
