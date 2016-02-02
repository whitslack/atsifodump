# atsifodump

atsifodump is a tool for dumping Audio Title Set (ATS) information from DVD-AUDIO discs, in several formats:

* Detailed technical dump
* Cue sheet for embedding in whole-album FLAC files
* Shell script for ripping tracks to individual FLAC files

Typically the ATS info will be found on a DVD-AUDIO disc at `/AUDIO_TS/ATS_01_0.IFO`. This is the DVD-AUDIO equivalent of the Video Title Set "IFO" files found on DVD-VIDEO discs. The ATS info structure differs somewhat from the VTS info structure, such that tools designed for DVD-VIDEO discs will not work on DVD-AUDIO discs.

Be advised that this tool does not decrypt the CPRM/CPPM encryption found on some DVD-AUDIO discs. However, the IFO files on these discs are not encrypted, so this tool can still parse them.

## Building

This tool is intended for use on Linux systems and makes use of certain GNU/Linux code constructs. However, it is mostly portable C99 code, so porting it to other platforms should not be too difficult.

To compile:

```
$ gcc -std=gnu99 -O3 -o atsifodump atsifodump.c
```

## Running

Run the tool with no arguments for a quick usage summary:

```
$ ./atsifodump
usage: ./atsifodump [-dcs] [-t<i>] ATS_xx_0.IFO
  -d    Output technical details (default)
  -c    Output cue sheet
  -s    Output shell script
  -t<i> Limit to program chain <i>
```

## Sample output

### Technical details

```
$ ./atsifodump /media/DVDVOLUME/AUDIO_TS/ATS_01_0.IFO
Audio Title Set Index (ATSI)
	ATS last sector: 1416328
	ATSI last sector: 1
	DVD spec version: 1.1
	PGCI first sector: 1
	Program Chain Index (PGCI)
		Program Chain #1
			Program count: 14
			Cell count: 14
			Duration: 321152100 (0:59:28.356+60)
			Program #1
				First PTS: 97 (0:00:00.001+7)
				Last PTS: 31408521 (0:05:48.983+51)
				Duration: 31408425 (0:05:48.982+45)
				First sector: 0
				Last sector: 132358
				Sector count: 132359
			Program #2
				First PTS: 31408522 (0:05:48.983+52)
				Last PTS: 54729921 (0:10:08.110+21)
				Duration: 23321400 (0:04:19.126+60)
				First sector: 132359
				Last sector: 225682
				Sector count: 93324
			Program #3
				First PTS: 54729922 (0:10:08.110+22)
				Last PTS: 71558571 (0:13:15.095+21)
				Duration: 16828650 (0:03:06.985)
				First sector: 225683
				Last sector: 292184
				Sector count: 66502
			Program #4
				First PTS: 71558572 (0:13:15.095+22)
				Last PTS: 101372271 (0:18:46.358+51)
				Duration: 29813700 (0:05:31.263+30)
				First sector: 292185
				Last sector: 410201
				Sector count: 118017
			Program #5
				First PTS: 101372272 (0:18:46.358+52)
				Last PTS: 121839471 (0:22:33.771+81)
				Duration: 20467200 (0:03:47.413+30)
				First sector: 410202
				Last sector: 491504
				Sector count: 81303
			Program #6
				First PTS: 121839472 (0:22:33.771+82)
				Last PTS: 155528646 (0:28:48.096+6)
				Duration: 33689175 (0:06:14.324+15)
				First sector: 491505
				Last sector: 632663
				Sector count: 141159
			Program #7
				First PTS: 155528647 (0:28:48.096+7)
				Last PTS: 166771521 (0:30:53.016+81)
				Duration: 11242875 (0:02:04.920+75)
				First sector: 632664
				Last sector: 683116
				Sector count: 50453
			Program #8
				First PTS: 166771522 (0:30:53.016+82)
				Last PTS: 184330296 (0:34:08.114+36)
				Duration: 17558775 (0:03:15.097+45)
				First sector: 683117
				Last sector: 768858
				Sector count: 85742
			Program #9
				First PTS: 184330297 (0:34:08.114+37)
				Last PTS: 211465371 (0:39:09.615+21)
				Duration: 27135075 (0:05:01.500+75)
				First sector: 768859
				Last sector: 884392
				Sector count: 115534
			Program #10
				First PTS: 211465372 (0:39:09.615+22)
				Last PTS: 226228296 (0:41:53.647+66)
				Duration: 14762925 (0:02:44.032+45)
				First sector: 884393
				Last sector: 947310
				Sector count: 62918
			Program #11
				First PTS: 226228297 (0:41:53.647+67)
				Last PTS: 237557871 (0:43:59.531+81)
				Duration: 11329575 (0:02:05.884+15)
				First sector: 947311
				Last sector: 997278
				Sector count: 49968
			Program #12
				First PTS: 237557872 (0:43:59.531+82)
				Last PTS: 250293771 (0:46:21.041+81)
				Duration: 12735900 (0:02:21.510)
				First sector: 997279
				Last sector: 1048316
				Sector count: 51038
			Program #13
				First PTS: 250293772 (0:46:21.041+82)
				Last PTS: 293723946 (0:54:23.599+36)
				Duration: 43430175 (0:08:02.557+45)
				First sector: 1048317
				Last sector: 1228902
				Sector count: 180586
			Program #14
				First PTS: 293723947 (0:54:23.599+37)
				Last PTS: 321152196 (0:59:28.357+66)
				Duration: 27428250 (0:05:04.758+30)
				First sector: 1228903
				Last sector: 1336948
				Sector count: 108046
		Program Chain #2
			Program count: 1
			Cell count: 1
			Duration: 23817375 (0:04:24.637+45)
			Program #1
				First PTS: 98 (0:00:00.001+8)
				Last PTS: 23817472 (0:04:24.638+52)
				Duration: 23817375 (0:04:24.637+45)
				First sector: 1336949
				Last sector: 1416308
				Sector count: 79360
```

### Cue sheet

```
$ ./atsifodump -c -t1 /media/DVDVOLUME/AUDIO_TS/ATS_01_0.IFO
FILE "" FLAC
  TRACK 01 AUDIO
    INDEX 01 00:00:00
  TRACK 02 AUDIO
    INDEX 01 05:48:73
  TRACK 03 AUDIO
    INDEX 01 10:08:08
  TRACK 04 AUDIO
    INDEX 01 13:15:07
  TRACK 05 AUDIO
    INDEX 01 18:46:26
  TRACK 06 AUDIO
    INDEX 01 22:33:57
  TRACK 07 AUDIO
    INDEX 01 28:48:07
  TRACK 08 AUDIO
    INDEX 01 30:53:01
  TRACK 09 AUDIO
    INDEX 01 34:08:08
  TRACK 10 AUDIO
    INDEX 01 39:09:46
  TRACK 11 AUDIO
    INDEX 01 41:53:48
  TRACK 12 AUDIO
    INDEX 01 43:59:39
  TRACK 13 AUDIO
    INDEX 01 46:21:03
  TRACK 14 AUDIO
    INDEX 01 54:23:44
```

### Shell script

```
$ ./atsifodump -s /media/DVDVOLUME/AUDIO_TS/ATS_01_0.IFO
#!/bin/sh
cat /media/DVDVOLUME/AUDIO_TS/ATS_01_?.AOB | {
	head -c271071232 | ffmpeg -i - Title01-Track01.flac
	head -c191127552 | ffmpeg -i - Title01-Track02.flac
	head -c136196096 | ffmpeg -i - Title01-Track03.flac
	head -c241698816 | ffmpeg -i - Title01-Track04.flac
	head -c166508544 | ffmpeg -i - Title01-Track05.flac
	head -c289093632 | ffmpeg -i - Title01-Track06.flac
	head -c103327744 | ffmpeg -i - Title01-Track07.flac
	head -c175599616 | ffmpeg -i - Title01-Track08.flac
	head -c236613632 | ffmpeg -i - Title01-Track09.flac
	head -c128856064 | ffmpeg -i - Title01-Track10.flac
	head -c102334464 | ffmpeg -i - Title01-Track11.flac
	head -c104525824 | ffmpeg -i - Title01-Track12.flac
	head -c369840128 | ffmpeg -i - Title01-Track13.flac
	head -c221278208 | ffmpeg -i - Title01-Track14.flac
	head -c162529280 | ffmpeg -i - Title02-Track01.flac
}
```

The output from `atsifodump -s` may be piped into `/bin/sh` to execute it.
