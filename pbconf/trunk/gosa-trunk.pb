#
# Project Builder configuration file
# For project gosa-trunk
#
# $Id$
#

#
# What is the project URL
#
#pburl gosa-trunk = svn://svn.gosa-trunk.org/gosa-trunk/devel
#pburl gosa-trunk = svn://svn+ssh.gosa-trunk.org/gosa-trunk/devel
pburl gosa-trunk = https://oss.gonicus.de/repositories/gosa-contrib/gosa-all
#pburl gosa-trunk = http://www.gosa-trunk.org/src/gosa-trunk-devel.tar.gz
#pburl gosa-trunk = ftp://ftp.gosa-trunk.org/src/gosa-trunk-devel.tar.gz
#pburl gosa-trunk = file:///src/gosa-trunk-devel.tar.gz
#pburl gosa-trunk = dir:///src/gosa-trunk-devel

# Repository
#pbrepo gosa-trunk = ftp://ftp.gosa-trunk.org
#pbml gosa-trunk = gosa-trunk-announce@lists.gosa-trunk.org
#pbsmtp gosa-trunk = localhost

# Check whether project is well formed 
# when downloading from ftp/http/...
# (containing already a directory with the project-version name)
#pbwf gosa-trunk = 1

#
# Packager label
#
pbpackager gosa-trunk = Benoit Mortier <benoit.mortier@opensides.be>
#

# For delivery to a machine by SSH (potentially the FTP server)
# Needs hostname, account and directory
#
#sshhost gosa-trunk = www.gosa-trunk.org
#sshlogin gosa-trunk = bill
#sshdir gosa-trunk = /gosa-trunk/ftp
#sshport gosa-trunk = 22

#
# For Virtual machines management
# Naming convention to follow: distribution name (as per ProjectBuilder::Distribution)
# followed by '-' and by release number
# followed by '-' and by architecture
# a .vmtype extension will be added to the resulting string
# a QEMU rhel-3-i286 here means that the VM will be named rhel-3-i386.qemu
#
#vmlist gosa-trunk = mandrake-10.1-i386,mandrake-10.2-i386,mandriva-2006.0-i386,mandriva-2007.0-i386,mandriva-2007.1-i386,mandriva-2008.0-i386,redhat-7.3-i386,redhat-9-i386,fedora-4-i386,fedora-5-i386,fedora-6-i386,fedora-7-i386,fedora-8-i386,rhel-3-i386,rhel-4-i386,rhel-5-i386,suse-10.0-i386,suse-10.1-i386,suse-10.2-i386,suse-10.3-i386,sles-9-i386,sles-10-i386,gentoo-nover-i386,debian-3.1-i386,debian-4.0-i386,ubuntu-6.06-i386,ubuntu-7.04-i386,ubuntu-7.10-i386,mandriva-2007.0-x86_64,mandriva-2007.1-x86_64,mandriva-2008.0-x86_64,fedora-6-x86_64,fedora-7-x86_64,fedora-8-x86_64,rhel-4-x86_64,rhel-5-x86_64,suse-10.2-x86_64,suse-10.3-x86_64,sles-10-x86_64,gentoo-nover-x86_64,debian-4.0-x86_64,ubuntu-7.04-x86_64,ubuntu-7.10-x86_64

#
# Valid values for vmtype are
# qemu, (vmware, xen, ... TBD)
#vmtype gosa-trunk = qemu

# Hash for VM stuff on vmtype
#vmntp default = pool.ntp.org

# We suppose we can commmunicate with the VM through SSH
#vmhost gosa-trunk = localhost
#vmlogin gosa-trunk = pb
#vmport gosa-trunk = 2222

# Timeout to wait when VM is launched/stopped
#vmtmout default = 120

# per VMs needed paramaters
#vmopt gosa-trunk = -m 384 -daemonize
#vmpath gosa-trunk = /home/qemu
#vmsize gosa-trunk = 5G

# 
# For Virtual environment management
# Naming convention to follow: distribution name (as per ProjectBuilder::Distribution)
# followed by '-' and by release number
# followed by '-' and by architecture
# a .vetype extension will be added to the resulting string
# a chroot rhel-3-i286 here means that the VE will be named rhel-3-i386.chroot
#
#velist gosa-trunk = fedora-7-i386

# VE params
#vetype gosa-trunk = chroot
#ventp default = pool.ntp.org
#velogin gosa-trunk = pb
#vepath gosa-trunk = /var/lib/mock
#veconf gosa-trunk = /etc/mock
#verebuild gosa-trunk = false

#
# Global version/tag for the project
#
#projver gosa-trunk = devel
#projtag gosa-trunk = 1

# Hash of valid version names

# Additional repository to add at build time
# addrepo centos-5-x86_64 = http://packages.sw.be/rpmforge-release/rpmforge-release-0.3.6-1.el5.rf.x86_64.rpm,ftp://ftp.project-builder.org/test/centos/5/pb.repo
# addrepo centos-5-x86_64 = http://packages.sw.be/rpmforge-release/rpmforge-release-0.3.6-1.el5.rf.x86_64.rpm,ftp://ftp.project-builder.org/test/centos/5/pb.repo
#version gosa-trunk = devel,stable

# Is it a test version or a production version
testver gosa-trunk = true

# Additional repository to add at build time
# addrepo centos-5-x86_64 = http://packages.sw.be/rpmforge-release/rpmforge-release-0.3.6-1.el5.rf.x86_64.rpm,ftp://ftp.project-builder.org/test/centos/5/pb.repo
# addrepo centos-4-x86_64 = http://packages.sw.be/rpmforge-release/rpmforge-release-0.3.6-1.el4.rf.x86_64.rpm,ftp://ftp.project-builder.org/test/centos/4/pb.repo

# Adapt to your needs:
# Optional if you need to overwrite the global values above
#
#pkgver gosa-trunk = stable
#pkgtag gosa-trunk = 3
# Hash of default package/package directory
defpkgdir gosa-trunk = gosa-all
# Hash of additional package/package directory
#extpkgdir minor-pkg = dir-minor-pkg

# List of files per pkg on which to apply filters
# Files are mentioned relatively to pbroot/defpkgdir
#filteredfiles gosa-trunk = Makefile.PL,configure.in,install.sh,gosa-trunk.8
#supfiles gosa-trunk = gosa-trunk.init

# For perl modules, names are different depending on distro
# Here perl-xxx for RPMs, libxxx-perl for debs, ...
# So the package name is indeed virtual
#namingtype gosa-trunk = perl
