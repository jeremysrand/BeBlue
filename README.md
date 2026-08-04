# BeBlue

**This is not production quality and you could lose data.  Use at own risk**

With that out of the way, this project is an attempt to bring BlueSCSI support to BeOS.
I am focused on the PPC platforms and my BeBox specifically, at least to start with.  Not too
many Intel based machines running BeOS are using SCSI so I don't think there is too much
value in an Intel build.  If you disagree, let me know.

## Features

There are a number of things you can do with BeBlue:
* Scan the SCSI bus(es) and identify all targets which are BlueSCSI devices
* Enable/disable debug mode on a BlueSCSI
* Get/set the current working directory of the BlueSCSI
* List the files and directories in a directory on the BlueSCSI
* Send files and directories from BeOS to the SD card of the BlueSCSI
* Get files and directories from the SD card of the BlueSCSI to your BeOS machine
* List the CDs available on an emulated CD drive on the BlueSCSI
* Switch to a different CD in the emulated CD drive on the BlueSCSI
* There is code to list and join WiFi networks but it doesn't work yet because it needs a kernel driver (see Future)

## CLI

The CLI usage looks like this:

```
USAGE: ./BeBlueCLI [-h] [-d device] [-v] [-r] [-f] [-l logfile] command...
  Commands:
    scan
    inquiry
    capabilities
    debug [on|off]
    devices
    working-dir [path]
    files [path]
    get source [dest]
    send source [dest]
    cds
    set-cd filename
    wifi-scan
    wifi-join channel ssid key
    wifi-info
```

The -h argument is short for help and shows the usage

The -d argument allows you to specify which device you want to interact with.  If not specified, the command
will scan the SCSI bus(es) and use the first BlueSCSI it finds.  But you can specify a path
like `/dev/bus/scsi/0/5/0/raw` using this argument.  You may need to do this if your BlueSCSI is providing
a hard drive at target 0 and a CD at target 5.  By default, the scan will choose the hard drive at target 0.
But if you want to list CDs, you will need to force it to use `/dev/bus/scsi/0/5/0/raw` for example.

The -v argument increases the verbosity of the output.

The -r argument is used with get and send commands and tells them to allow recursion.  You must specify
the -r argument if you want to get or send directories.

The -f argument is only used with the get and send commands.  If you are getting a file or directory from the
BlueSCSI and copying it onto BeOS and the target file/dirs already exist, the command will fail.  But if you use
the -f argument to force the get, the existing file/dir will be deleted and the get will proceed.  Similarly,
if you are sending a file or directory to the BlueSCSI and that file or directory already exists, the command
will fail.  Use the -f argument to force the send.  Note that sending a directory to an existing directory
merges the existing contents of the directory on the BlueSCSI with the files from the source.  There is no way
to delete files from the BlueSCSI today.  Even so, be very careful with the force option.  You can lose data.

The -l argument allows you to log detailed information.  If you think you have found a bug, reproduce it and
use the -l argument to gather the logs.  You can create an issue here with the logs attached.

The individual commands are described below

### scan

The scan command walks all devices on all SCSI busses and identifies all devices.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI scan
Scanning for BlueSCSI devices on all SCSI busses:

+------------------------------------------------------------+
| Bus | ID | LUN | Type    | Vendor  | Device          | Rev |
|------------------------------------------------------------|
| 0   | 0  | 0   | Disk    | BLUESCSI| HARDDRIVE       | 1.0 |
| 0   | 5  | 0   | CD-ROM  | BLUESCSI| CDROM           | 1.0 |
+------------------------------------------------------------+
```

### inquiry

The inquiry command dumps the output of a SCSI inquiry command against the selected
device.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI inquiry
Inquiry output from /dev/bus/scsi/0/0/0/raw:
  Type:         0 (Disk)
  SCSI Version: 2
  Vendor:       "BLUESCSI"
  Device:       "HARDDRIVE"
  Version:      "1.0"
```

### capabilities

The capabilities command prints the state of the BlueSCSI's extra features.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI capabilities
Capabilities of /dev/bus/scsi/0/0/0/raw:
    Version: 0
    Flags:   7
      Large Transfers: Supported
      Large Send:      Supported
      Set Working Dir: Supported

```

### debug

The debug command allows you to check to see if debugging is on or enable/disable
debugging on the BlueSCSI.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI debug
/dev/bus/scsi/0/0/0/raw:
  Debug: Disabled
[/boot/home/code/BeBlue]> ./BeBlueCLI debug on
/dev/bus/scsi/0/0/0/raw:
  Debug: Enabled
[/boot/home/code/BeBlue]> ./BeBlueCLI debug
/dev/bus/scsi/0/0/0/raw:
  Debug: Enabled
[/boot/home/code/BeBlue]> ./BeBlueCLI debug off
/dev/bus/scsi/0/0/0/raw:
  Debug: Disabled
[/boot/home/code/BeBlue]> ./BeBlueCLI debug
/dev/bus/scsi/0/0/0/raw:
  Debug: Disabled
```

### devices

The devices command asks the BlueSCSI for information about all of the individual
devices on the SCSI bus that it is emulating.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI devices
Devices on /dev/bus/scsi/0/0/0/raw:

+------------------------------------+
| ID | Type  | Description           |
|------------------------------------|
| 0  | 0x00  | Fixed Disk            |
| 1  | 0xff  | No Device             |
| 2  | 0xff  | No Device             |
| 3  | 0xff  | No Device             |
| 4  | 0xff  | No Device             |
| 5  | 0x02  | Optical Disk          |
| 6  | 0xff  | No Device             |
| 7  | 0xff  | No Device             |
+------------------------------------+
```

### working-dir

The working-dir command allows you to get or set the working directory that the BlueSCSI
is using on the SD card.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI working-dir
/dev/bus/scsi/0/0/0/raw:
  Working Dir: /shared
[/boot/home/code/BeBlue]> ./BeBlueCLI working-dir /testing
/dev/bus/scsi/0/0/0/raw:
  Working Dir: /testing
[/boot/home/code/BeBlue]> ./BeBlueCLI working-dir
/dev/bus/scsi/0/0/0/raw:
  Working Dir: /testing
```

### files

The files command lists the files in a directory on the SD card.  If you do not provide
a directory, it shows a list from the current working directory.  If you do provide a
direcotry, it shows the list from that directory but leaves the current working
directory unchanged.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI files
List files under /shared on /dev/bus/scsi/0/0/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | D    | 262144        | common                           |
| 1     | D    | 262144        | foo                              |
| 2     | F    | 1078          | LICENSE                          |
| 3     | F    | 1078          | outfile                          |
+-----------------------------------------------------------------+
[/boot/home/code/BeBlue]> ./BeBlueCLI files /
List files under / on /dev/bus/scsi/0/0/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | F    | 21943         | log.txt                          |
| 1     | D    | 262144        | InstallCDs                       |
| 2     | D    | 262144        | shared                           |
| 3     | F    | 1252          | lastlog.txt                      |
| 4     | F    | 4294967296    | HD0 BeOS5.hda                    |
| 5     | D    | 262144        | ToDo                             |
| 6     | D    | 262144        | CD5                              |
| 7     | D    | 262144        | HDs                              |
| 8     | D    | 262144        | testing                          |
| 9     | F    | 156           | bluescsi.ini                     |
| 10    | D    | 262144        | test2                            |
+-----------------------------------------------------------------+
```

### get

The get command fetches files and directories from the BlueSCSI and writes them
to your BeOS system.  You may want to use the -r or the -f arguments to enable
recurse or force mode or both.  If you do not specify a destination, then the name
of the file or directory you are getting will be written at the current working
directory of your shell on yout BeOS system.

```
[/boot/home/code/BeBlue]> ls -l file
/bin/ls: file: No such file or directory
[/boot/home/code/BeBlue]> ./BeBlueCLI get /test2/file
[/boot/home/code/BeBlue]> ls -l file
-rw-r--r--   1 jrand    bedev        1078 Aug  1 19:43 file
[/boot/home/code/BeBlue]> ls -al foo
/bin/ls: foo: No such file or directory
[/boot/home/code/BeBlue]> ./BeBlueCLI -r get /test2/dir foo
[/boot/home/code/BeBlue]> ls -al foo
total 67
drwxr-xr-x   1 jrand    bedev        2048 Aug  1 19:44 .
drwxr-xr-x   1 jrand    bedev        2048 Aug  1 19:44 ..
-rw-r--r--   1 jrand    bedev       17671 Aug  1 19:44 BlueSCSICommand.cpp
-rw-r--r--   1 jrand    bedev        3784 Aug  1 19:44 BlueSCSICommand.h
-rw-r--r--   1 jrand    bedev        5107 Aug  1 19:44 BlueSCSIDevice.cpp
-rw-r--r--   1 jrand    bedev        1270 Aug  1 19:44 BlueSCSIDevice.h
-rw-r--r--   1 jrand    bedev       10618 Aug  1 19:44 BlueSCSIGet.cpp
-rw-r--r--   1 jrand    bedev        1389 Aug  1 19:44 BlueSCSIGet.h
-rw-r--r--   1 jrand    bedev        1265 Aug  1 19:44 BlueSCSIScan.cpp
-rw-r--r--   1 jrand    bedev         527 Aug  1 19:44 BlueSCSIScan.h
-rw-r--r--   1 jrand    bedev        8140 Aug  1 19:44 BlueSCSISend.cpp
-rw-r--r--   1 jrand    bedev        1310 Aug  1 19:44 BlueSCSISend.h
-rw-r--r--   1 jrand    bedev          61 Aug  1 19:44 Common.h
-rw-r--r--   1 jrand    bedev         443 Aug  1 19:44 Logger.cpp
-rw-r--r--   1 jrand    bedev         308 Aug  1 19:44 Logger.h
-rw-r--r--   1 jrand    bedev        8009 Aug  1 19:44 SCSICommand.cpp
-rw-r--r--   1 jrand    bedev        1383 Aug  1 19:44 SCSICommand.h  
```

### send

The send command copies file from your BeOS system to the SD card of the BlueSCSI.
You may want to use the -r argument to enable recursive mode if you want to send
a directory.  If you do not specify a destination, then the name of the file or
directory you are sending to the SD card will be written to the current working
directory of the BlueSCSI

```
[/boot/home/code/BeBlue]> ./BeBlueCLI working-dir /newdir
/dev/bus/scsi/0/0/0/raw:
  Working Dir: /newdir
[/boot/home/code/BeBlue]> ./BeBlueCLI files
List files under /newdir on /dev/bus/scsi/0/0/0/raw:
  No files found.
[/boot/home/code/BeBlue]> ./BeBlueCLI send LICENSE
[/boot/home/code/BeBlue]> ./BeBlueCLI files
List files under /newdir on /dev/bus/scsi/0/0/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | F    | 1078          | LICENSE                          |
+-----------------------------------------------------------------+
[/boot/home/code/BeBlue]> ./BeBlueCLI -r send common codedir
[/boot/home/code/BeBlue]> ./BeBlueCLI files
List files under /newdir on /dev/bus/scsi/0/0/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | F    | 1078          | LICENSE                          |
| 1     | D    | 262144        | codedir                          |
+-----------------------------------------------------------------+
[/boot/home/code/BeBlue]> ./BeBlueCLI files /newdir/codedir
List files under /newdir/codedir on /dev/bus/scsi/0/0/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | F    | 17671         | BlueSCSICommand.cpp              |
| 1     | F    | 3784          | BlueSCSICommand.h                |
| 2     | F    | 5107          | BlueSCSIDevice.cpp               |
| 3     | F    | 1270          | BlueSCSIDevice.h                 |
| 4     | F    | 10749         | BlueSCSIGet.cpp                  |
| 5     | F    | 1389          | BlueSCSIGet.h                    |
| 6     | F    | 1265          | BlueSCSIScan.cpp                 |
| 7     | F    | 527           | BlueSCSIScan.h                   |
| 8     | F    | 8140          | BlueSCSISend.cpp                 |
| 9     | F    | 1310          | BlueSCSISend.h                   |
| 10    | F    | 61            | Common.h                         |
| 11    | F    | 443           | Logger.cpp                       |
| 12    | F    | 308           | Logger.h                         |
| 13    | F    | 8009          | SCSICommand.cpp                  |
| 14    | F    | 1383          | SCSICommand.h                    |
+-----------------------------------------------------------------+ 
```

### cds

The cds command lists the CDROM images that are available on a emulated CD drive.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI -d /dev/bus/scsi/0/5/0/raw cds
List CDs on /dev/bus/scsi/0/5/0/raw:

+-----------------------------------------------------------------+
| Index | Type | Size          | Name                             |
|-----------------------------------------------------------------|
| 0     | F    | 545712128     | CD5 BeOS_R3.iso                  |
| 1     | F    | 673261568     | CD5 BeOS_R4_5.iso                |
| 2     | F    | 643682304     | CD5 BeOS_R4.iso                  |
| 3     | F    | 642168832     | CD5 BeOS_R5.bin                  |
+-----------------------------------------------------------------+
```

### set-cd

The set-cd command allows you to change to a different CD image on an emulated CD drive.

```
[/boot/home/code/BeBlue]> ./BeBlueCLI -d /dev/bus/scsi/0/5/0/raw set-cd "CD5 BeOS_R4_5.iso"
```

### wifi-scan

The wifi-scan command should list all available WiFi networks.  But this command does
not work yet (see Future).


### wifi-join

The wifi-join command alows you to connect to a specific WiFi network.  But this
command does not work yet (see Future).


### wifi-info

The wifi-info command shows details about the currently joined WiFi network.  But this
command does not work yet (see Future).


## Future

In the future, I would like to add support for:
* A GUI with support for drag and drop to make file operations easy.
* WiFi network support.  I have already written the code to scan for WiFi networks and join them.  What I need to do to actually support this is write a kernel driver for the packet interface.