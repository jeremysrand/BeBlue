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

The -f argument is only used with the get command.  If you are getting a file or directory from the BlueSCSI
and copying it onto BeOS and the target file/dirs already exist, the command will fail.  But if you use the -f
argument to force the get, the existing file/dir will be deleted and the get will proceed.  Be careful with this
option.

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


## Future

In the future, I would like to add support for:
* A GUI with support for drag and drop to make file operations easy.
* WiFi network support.  I have already written the code to scan for WiFi networks and join them.  What I need to do to actually support this is write a kernel driver for the packet interface.