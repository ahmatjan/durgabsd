# Nessus add-on for Dradis

[![Build Status](https://secure.travis-ci.org/dradis/dradis-nessus.png?branch=master)](http://travis-ci.org/dradis/dradis-nessus) [![Code Climate](https://codeclimate.com/github/dradis/dradis-nessus.png)](https://codeclimate.com/github/dradis/dradis-nessus.png)

The Nessus upload add-on will enable user to upload Nessus output files in the nessus client format (.nessus) to create a structure of nodes/notes that contain the same information about the hosts/ports/services as the original file.

The parser only supports version 2 of nessus xml format. Other formats (nbe, nsr) are not supported at the moment.

Also, the xml parser only extracts the results of a scan. It is not able to parse the scan policy itself which is also part of the xml file.

The add-on requires Dradis 3.0 or higher.


## More information

See the Dradis Framework's [README.md](https://github.com/dradis/dradisframework/blob/master/README.md)


## Contributing

See the Dradis Framework's [CONTRIBUTING.md](https://github.com/dradis/dradisframework/blob/master/CONTRIBUTING.md)


## License

Dradis Framework and all its components are released under [GNU General Public License version 2.0](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) as published by the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
