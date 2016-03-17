# EUBrazilCC UC3 Elastic Engine

In order to compile and run the EUBrazilCC UC3 Elastic Engine, make sure you have the following packages (all available through CentOS official repositories, the epel repository and MySQL RHEL 6 repository) properly installed:

1. jansson and jansson-devel
2. mysql-community-server and mysql-community-libs

Furthermore, you need an ophidia-terminal to run jobs and a configured MySQL server to manage the System Catalog. 

### How to Install

To build you need automake, autoconf and libtool packages. To prepare the code for building run:

```
$ ./bootstrap 
```

To install simply type:

```
$ ./configure --prefix=prefix
$ make
$ make install
```

Type:

```
$ ./configure --help
```

to see all available options.

