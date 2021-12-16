# CogGroup-MAC

CogGroup-MAC based on NS-3.33





### Preparation for installation of ns-3.33

The following list of packages should be accurate through the Ubuntu  21.04 release; other releases or other Debian-based systems may slightly vary.  Ubuntu 16.04 LTS release is probably the oldest release that is  known to work with recent ns-3 releases.

**Note:** As of ns-3.30 release (August 2019), ns-3 uses  Python 3 by default, but earlier releases depend on Python 2 packages,  and at least a Python 2 interpreter is recommended.  If working with an  earlier release, one may in general substitute 'python' for 'python3' in the below (e.g. install 'python-dev' instead of 'python3-dev').

-  **minimal requirements for C++ users (release):**  This is the minimal set of packages needed to run ns-3 from a released tarball.  

```
 apt install g++ python3
```

-  **minimal requirements for Python API users (release 3.30 and newer, and ns-3-dev):** This is the minimal set of packages needed to work with Python bindings from a released tarball.

```
 apt install g++ python3 python3-dev pkg-config sqlite3
```

-  **minimal requirements for Python (development):** For use  of ns-3-allinone repository (cloned from Git), additional packages are  needed to fetch and successfully install pybindgen and netanim.

```
 apt install python3-setuptools git
```

-  **Netanim animator:**  qt5 development tools are needed for Netanim animator; qt4 will also work but we have migrated to qt5.

```
 apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

**Note:** For Ubuntu 20.10 and earlier, the single 'qt5-default' package suffices

```
 apt install qt5-default
```

-  Support for ns-3-pyviz visualizer

- -  For Ubuntu 18.04 and later, python-pygoocanvas is no  longer provided.  The ns-3.29 release and later upgrades the support to  GTK+ version 3, and requires these packages:

```
 apt install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3  
```

- -  For ns-3.28 and earlier releases, PyViz is based on GTK+ 2, GooCanvas, and GraphViz:

```
 apt install python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython
```

-  Support for MPI-based distributed emulation

```
apt install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```

-  Support for bake build tool:

```
 apt install autoconf cvs bzr unrar
```

-  Debugging:

```
 apt install gdb valgrind 
```

-  Support for utils/check-style.py code style check program

```
apt install uncrustify
```

-  Doxygen and related inline documentation:

```
 apt install doxygen graphviz imagemagick
 apt install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
```

- 
  -  If you get an error such as 'convert ... not authorized  source-temp/figures/lena-dual-stripe.eps', see this post about editing  ImageMagick's security policy configuration: https://cromwell-intl.com/open-source/pdf-not-authorized.html.  In brief, you will want to make this kind of change to ImageMagick security policy:

```
   --- ImageMagick-6/policy.xml.bak	2020-04-28 21:10:08.564613444 -0700
   +++ ImageMagick-6/policy.xml	2020-04-28 21:10:29.413438798 -0700
   @@ -87,10 +87,10 @@
      <policy domain="path" rights="none" pattern="@*"/>
   -  <policy domain="coder" rights="none" pattern="PS" />
   +  <policy domain="coder" rights="read|write" pattern="PS" />
      <policy domain="coder" rights="none" pattern="PS2" />
      <policy domain="coder" rights="none" pattern="PS3" />
      <policy domain="coder" rights="none" pattern="EPS" />
   -  <policy domain="coder" rights="none" pattern="PDF" />
   +  <policy domain="coder" rights="read|write" pattern="PDF" />
      <policy domain="coder" rights="none" pattern="XPS" />
    </policymap>
```



-  The ns-3 manual and tutorial are written in reStructuredText  for Sphinx (doc/tutorial, doc/manual, doc/models), and figures typically in dia (also needs the texlive packages above):

```
 apt install python3-sphinx dia 
```

**Note:** Sphinx version >= 1.12 required for ns-3.15.  To  check your version, type "sphinx-build".  To fetch this package alone,  outside of the Ubuntu package system, try "sudo easy_install -U Sphinx".

-  GNU Scientific Library (GSL) support for more accurate 802.11b WiFi error models (not needed for OFDM):

```
 apt install gsl-bin libgsl-dev libgslcblas0
```

If the above doesn't work (doesn't detect GSL on the system), consult: https://coral.ise.lehigh.edu/jild13/2016/07/11/hello/.  But don't worry if you are not using 802.11b models.

-  To read pcap packet traces

```
apt install tcpdump
```

-  Database support for statistics framework

```
apt install sqlite sqlite3 libsqlite3-dev
```

-  Xml-based version of the config store (requires libxml2 >= version 2.7)

```
apt install libxml2 libxml2-dev
```

-  Support for generating modified python bindings 

```
 apt install cmake libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
 python3 -m pip install --user cxxfilt
```

and you will want to install castxml and pygccxml as per the instructions for python bindings (or through the *bake* build tool as described in the tutorial).  The 'castxml' and 'pygccxml' packages provided by Ubuntu 18.04 and earlier are not recommended; a  source build (coordinated via bake) is recommended.  If you plan to work with bindings or rescan them for any ns-3 C++ changes you might make,  please read the [chapter in the manual](https://www.nsnam.org/docs/manual/html/python.html) on this topic.

**Note:** Ubuntu versions (through 19.04) and systems based on it (e.g. Linux Mint 18) default to an old version of clang and llvm  (3.8), when simply 'libclang-dev' and 'llvm-dev' are specified.     The  packaging on these 3.8 versions is broken.  Users of Ubuntu will want to explicitly install a newer version by specifying 'libclang-6.0-dev' and 'llvm-6.0-dev'.  Other versions newer than 6.0 may work (not tested).

-  A GTK-based configuration system

```
 apt install libgtk-3-dev
```

-  To experiment with virtual machines and ns-3

```
 apt install vtun lxc uml-utilities
```

-  Support for openflow module (requires libxml2-dev if not installed above) and Boost development libraries

```
apt install libxml2 libxml2-dev libboost-all-dev
```

### Downloading a release of ns-3 as a source archive

Type the following:

```
$ cd
$ mkdir workspace
$ cd workspace
$ wget https://www.nsnam.org/release/ns-allinone-3.33.tar.bz2
$ tar xjf ns-allinone-3.33.tar.bz2
```

Following these steps, if you change into the directory `ns-allinone-3.33`, you should see a number of files and directories

```
$ cd ns-allinone-3.33
$ ls
bake      constants.py   ns-3.33                            README
build.py  netanim-3.108  pybindgen-0.21.0                   util.py
```

### Building with `build.py`

Change into the directory `ns-allinone-3.33` under your `~/workspace` directory. Type the following:

```
$ ./build.py --enable-examples --enable-tests
```

### Building with Waf

Up to this point, we have used either the build.py script, or the bake tool, to get started with building *ns-3*.  These tools are useful for building *ns-3* and supporting libraries, and they call into the *ns-3* directory to call the Waf build tool to do the actual building. An installation of Waf is bundled with the *ns-3* source code. Most users quickly transition to using Waf directly to configure and build *ns-3*.  So, to proceed, please change your working directory to the *ns-3* directory that you have initially built.

Now go ahead and switch back to the debug build that includes the examples and tests.

```
$ ./waf clean
$ ./waf configure --build-profile=debug --enable-examples --enable-tests
```

The build system is now configured and you can build the debug versions of the *ns-3* programs by simply typing:

```
$ ./waf
```

Although the above steps made you build the *ns-3* part of the system twice, now you know how to change the configuration and build optimized code.



### Extending CogGroup-MAC Module

Copy and paste folder `~/CogGroup-MAC/src/`  in `~/CogGroup-MAC/` into `~/workspace/ns-allinone-3.33/ns-3.33` directory, This step will overwrite some files.

```
$ cp -r ./CogGroup-MAC/src/* ~/workspace/ns-allinone-3.33/ns-3.33/src/
```

### Rebuilding with Waf

Type the following:

```
$ ./waf clean
$ ./waf configure --build-profile=debug --enable-examples --enable-tests
$ ./waf
```

### Running an example of CogGroup-MAC

#### Running the `ns_stript`

First, we recompile ns-2.33 with the following command

```
$ cd ~/workspace/ns-allinone-3.33/ns-3.33/
$ ./waf 
```

Run the `ns_stript`

```
$ ./waf --run scratch/ns_stript
```



