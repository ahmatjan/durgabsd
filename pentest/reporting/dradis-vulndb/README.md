# VulnDB HQ add-on for Dradis

[![Build Status](https://secure.travis-ci.org/dradis/dradis-vulndb.png?branch=master)](http://travis-ci.org/dradis/dradis-vulndb) [![Code Climate](https://codeclimate.com/github/dradis/dradis-vulndb.png)](https://codeclimate.com/github/dradis/dradis-vulndb.png)


This add-on can be used to connect the Dradis Framework to a remote vulnerability database such as VulnDB HQ (http://vulndbhq.com/). The idea is to have a database with commonly used notes that you can import to your current project. 

The current version of the add-on uses the [vulndbhq gem](https://github.com/securityroots/vulndbhq) to access the service API

You can also use the Thor task to search Vuln::DB from the command line:

   $ thor dradis:import:vulndb:search XSS

The add-on requires Dradis 3.0 or higher

## More information

See the Dradis Framework's [README.md](https://github.com/dradis/dradisframework/blob/master/README.md)


## Contributing

See the Dradis Framework's [CONTRIBUTING.md](https://github.com/dradis/dradisframework/blob/master/CONTRIBUTING.md)


## License

Dradis Framework and all its components are released under [GNU General Public License version 2.0](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) as published by the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
