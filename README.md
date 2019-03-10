# ArchSimian
You worked hard to build a large music library (2,000+ rated tracks), so make the most out of it with ArchSimian!

Improve management of a music library built with MediaMonkey 4 (MM4) for Windows by integrating MM4 database data with an Arch Linux program. Adds more advanced playlist building features than MM4 currently offers, specifically to better control variety and frequency of play than MM4’s smart playlists provides. Users import a playlist from MM4, add tracks using ArchSimian, then export the improved playlist back into MM4 for use in Windows or to sync with mobile devices using the MM4 for Android App. They will also be able to use the same playlists in Audacious or other Linux-based media players, using a shared music library.

<b>Improving MediaMonkey 4 Gold by integrating it with Arch Linux</b>


Why would anyone want to do this, and how can Arch make MediaMonkey 4 Gold (MM4) better than it already is? 

It depends on whether you have a large music library, and whether you are happy with the smart playlist features in MM4. 

You may want this program if you have a large music library and want to build a more complex set of rules for populating a playlist to have a more refined selection of tracks, with better variety.

For example, let's say you have two 'different' artists, "Tom Petty" and "Tom Petty & the Heartbreakers." This is basically the same artist and so you wish to treat them both as the same artist for purposes of when the artist should be repeated. You store a common name for both (eg. the custom grouping "Petty") in MM4 (using each tag's "Custom2" field), then, ArchSimian will treat either name as the same artist. With this in mind, you can create any custom artist groups you like, such as "Pop1971" with different artists. It is up to you.

While MM4 has many great smart playlist features built in (and far better than any other retail package out there), it does not provide the complexity or speed with which you can build them in C++ (and, a big benefit of using Arch and KDE is having some added programs/scripts to tag elements the way I want them, and have them integrated right onto my desktop, using KDE and Conky. I can update tag elements or add art work using convenient shortcuts, and update them in real time.)

<b>What does ArchSimian do?</b>

<b>It creates refined playlists using existing data from your MediaMonkey database file.</b>

The playlist improvement concept is this: the play history in the MM database can be used to compute a user’s average listening time per day. By also calculating the number of rated tracks, along with the ratings and playing times for each, it can be determined how frequently a song should be added for a repeat play onto the playlist. While MM4 already has a smart playlist feature for “days since last played” by rating, it does not do so with a high degree of specialization for variety, such as the arrangement of tracks within the list.

For example, this program lets you set the repeat frequency for your tracks by rating catgory. For example, you might like 60 days for 5 stars, 180 days for four stars, etc. MM4 smart playlists can do this, but it does not consider the specific order of play of the individual artists within it. 

<b>(TL;DR) Quickstart:</b>

1. Rate all tracks in MediaMonkey you want to include in this program. You should have 2,000 or more for it to work well. MediaMonkey should be "auto-organizing" your library and filenames should not have spaces in them. Your music library dir tree should look like ->  Drive:/artist/album/song in Windows.

2.  A one-star rating is special in Archsimian. Assign one-star in MediaMonkey for new tracks you have not yet rated. This tells ArchSimian to include them in the selection process. 

3. Assign zero stars in MediaMonkey to exclude tracks from this program.

4. You can set custom artist groupings in MediaMonkey using the "Custom2" field. ArchSimian will override artist tags wherever Custom2 is used.

5. Export the MM.DB (safest is to use a backup) to a dir you will select in ArchSimian setup menu. Also select the top dir of your music library, and folder where you export any playlists from MediaMonkey. Export playlists for use in ArchSimian using the "manual export" function found under MediaMonkey's "File" menu.

6. ArchSimian exports the modified playlist back to the dir where it was originally selected, and overwrites the older playlist.You will have a Windows-compatible playlist that can be imported by MediaMonkey.

<b>The primary goal is to improve the variety of artists within the playlist, while at the same time considering rating and lastplayed date.</b> 


    • Obtains music tag data and other data stored within the MM.DB file to create a new database for
    this application
    • Using the database created, and statistics compiled from it, uses the data as constraints to 
    determine ‘optimum availability’ then iteratively select each single track added to the chosen 
    playlist
    • Saves user preferences, such as file locations for MM4 backup database, MM4 backup playlists 
    and Archsimian program/playlist preferences
    • Obtains or calculates variables for calculating which track will be selected to add to the playlist: 
            ▪     user’s listening rate (for total tracks needed to be added to the current playlist)
            ▪     song rating (to determine frequency played)  
            ▪     days since last played (to use with rating)
            ▪     how many tracks ago was the last appearance of an artist (to avoid repeat artists)
            ▪     total number of rated tracks for each artist 
            ▪     total number of each rating category in the library     
            ▪     total number of each rating category by artist 
            ▪     last album played for a given artist

For artists with many rated songs, the “availability” of that artist may be more frequent, but without this program, you might end up with a song from the same artist after only 10 tracks, instead of 60 or 150. The spacing is set automatically based on the total number of rated tracks for each artist, and the track total is further adjusted based on the ratings assigned to each track. 

Also, when that artist is repeated you will hear a track from a different album than the one last played based on criteria you set.  

Replay intervals – you can set the planned time between repeats based on rating. By using ratios of repeat frequency with average listening time, the program can determine track should be added to the playlist. Variety takes precedence over repeat frequency.  If you set two-star tracks to repeat every three years, but your actual history is they have all been heard in the last two years, they will still be added based on the 'relative frequency' to that of the other tracks. To 'transition to longer times' set at a lower repeat frequency than what you desire (need three years, set to four years).

<b>Minimum Computer Requirements:</b>

8GB of RAM (You will need this much if you need to run KDE and the Windows VM smoothly. Less might work with a lighter DE (but no guarantees). Less will than 8 will be very choppy with RAM-hungry KDE. If you do not use the VM, 4GB is enough.

IF you use the VM, a CPU capable of running Windows 10 on an Oracle VirtualBox Virtual Machine (VM), meaning a 64-bit processor with hardware virtualization support enabled. See here for details: <html>https://forums.virtualbox.org/viewtopic.php?t=58072</html>

A Linux OS (I have tested it in Arch, for other distros, I have not tested, and highly recommend KDE as your Desktop Environment (DE), mainly for the notifications it gives you; also conky if you want your music player integrated onto the desktop itself). You may be able to make this work with other Linux distros or DEs, but your mileage will vary and any changes needed are not addressed here. 

MediaMonkey4 Gold for Windows 

(Note: while it could installed be on a separate partition of Windows using a dual boot system, it is far easier installed and running in a Windows 10 VM running in Arch. Even if you have Windows installed on a dual boot, you will want to also have the VM installed in Arch if you want to avoid having to reboot twice for every library/playlist update). 

Separately, you need to install/setup several scripts (.mmip files) in MM4:
 
Backup 6.0 (trixmoto) for backing up the MM4 database; 
Update Location of Files in Database; 
RegEx Find and Replace; and, 
ImportM3U

<b>Linux applications you will need (For Arch, via the AUR):</b>

SQLite 3 for database commands

<b>Linux applications you will likely want to use along with this program:</b>

audacious-qt5 (linux media player that sounds great, and best of all integrates with conky perfectly). Install audacious-plugins-qt5 and apulse.

conky (if you wish to display cover art and most other tag elements right on a panel docked onto your desktop (cool!)

id3ted for tag scripting requirements if you want to update tags in Linux

Oracle VirtualBox VM (to run MM4 in Linux by running Windows 10 on a VM)

<b>Preparation</b>

<b>1) Operating Systems</b> - If you have multiple drives (or are currently using MM4 on Windows on the same computer), decide whether you want Arch installed on the same box. If you do, the best practice is for the Arch drive to be on a separate hard drive from that of Windows (for security reasons).

Whether you do this or not, you will want to use a VM in Arch to update music tags and the MM4 database between the two operating systems if you do not wish to do constant reboots to change OSes. 

Your music library should contain at least <b> 2,000 rated tracks </b> and be located on a windows-formatted storage drive that will be shared by both your Windows VM and Arch so do not move your music library to a Linux-formatted drive. Set Arch file permissions accordingly for read-write access to the music library location. It may run on fewer rated tracks, but that has not been tested.

When installing the VM you have to add guest additions before you install MM4 so that you can configure access to the music library and the folder(s) you will use to store the MM4 backups for its database and for your playlists.


<b>2) MM4 installation/configuration</b>

This program requires prior preparation in MM4. Backup your MM4.DB and .ini files before you start. 

If you decide the program is not working for you, you can restore it back to your original configuration, but...WARNING: you will be “stuck” with any changes made to your library filename/directory structure as described below, so if you are using some other filenaming/directory approach, either be prepared to backup your whole music library with the original filenaming/directory configuration, or accept the alternative of doing the “MM4 Configuration” section below then doing a second backup of the db and ini files. Remember all of this is at your own risk!

Even if you choose to not use ArchSimian, the below configuration changes are useful for configuring your library to access it via your Linux distribution. The below configuration worked for me:

MM4 Configuration – Under Tools > Options > Library, enable the following: Infer file properties from filenames, use Tags then inferred properties; Get album titles and track numbers for tags only; Scan file directories for artwork; Update file info from tags when rescanning files. Under  Tools > Options > Library > Tags & Playlists, enable ID3v2 tags; ASCII Always; Update tags when editing properties; enable ‘Use Unicode (UTF-8 encoding) for M3U playlists, Warn when duplicate files are added; for artwork, Save image to file folder, First image per album/series: folder.jpg.

Tools > Options > Library > Appearance > Ignore prefixes

Library - ensure the file hierarchy of your MM4 music library drive conforms to the hierarchy of [drive]/artist/album, with no additional subfolders. All music folders and files should be in lower case and have no spaces or special characters (filename example: 01_-_alexi_murdoch_-_towards_the_sun.mp3).  To do file and folder naming, first go to Tools > AutoOrganize Files, and select the radio button for “Move and rename files to new destination based on file tags.” This lets MM4 manage your directory structure. Then, under destination, where “X” is the drive letter assigned to your music library by the VirtualBox VM guest addition. Enter this line, which will ensure the filenames are lower case letters and they do not have spaces:

X:\$Replace($lower(<Artist>), ,_)\$Replace($lower(<Album>), ,_)\<Track#:2>_-_$Replace($lower(<Artist>), ,_)_-_$Replace($lower(<Title>), ,_)

More on AutoOrganize filename changing here: <html>https://www.mediamonkey.com/sw/webhelp/frame/index.html?configuringdirectoryandfileformats.htm</html>

MM4 Installation in the VM – Install MM4 Gold and enter your license verification, then install .mmip scripts for Backup 6.0, Update Location of Files in Database, and ImportM3U. To use your original MM.DB and ini files in the VM, locate them, back them up and (with VM version of MM4 closed) copy it to the VM location replacing the one created at install, then open MM4 in the VM and modify the file location pointers. To do that, you can use the script “Update Location of Files in Database.” If you set up guest additions in the VM, you should have the new drive number. Select the old and new paths, then check “Update paths in database only for files that exist at new location.” Do not copy files.

If you use the MM4 Android App to sync to your phone wirelessly, you can still use it while MM4 is running in the VM, but you may need to disable KDE from autoconnecting to your phone so the VM can recognize the device.

The only issues I found have to do with non-compliance to the above filename standard, such as spaces or song files with special characters, or problems with some of the tags. I have not written debugging code for this (yet). Special characters within the music tags can also be fixed in MM4. You can search in MM4 for tags containing special characters by using this search format:

 '/', '\', '?', ',', '|', ':', '.', '_', '(', ')', '[', ']', '&', '@', '#', '+', '*', '!', '-', ';' '”' 

Unfortunately, it does not work for double quotation marks, which have to be changed, and you also will have to ensure no tags have carats (^) in them. So, the remedy is to install the .mmip “RegEx Find and Replace” (add on), then select the option for “Replace specified string with another one in <Into Field>...” You can then replace all instances of double quotation marks with single quotation marks for all tag fields (in particular you will find this occurrence in song titles and album titles) in your library. Carats need to be removed or replaced with a different character because it is the delimiter this program uses to extract the songs table from the MM.DB. If you have carats, the fields will not parse correctly in this program. It appears commas and single quotes do not cause any problem.

As referenced under MM4 configuration, artwork files must be stored as a single image in each album folder using the filename folder.jpg.

You need to ensure all tracks in MM4 you want to include in your playlists are rated prior to running Archsimian. 

Tracks with no rating are excluded from playlist computation in Archsimian, so for any new tracks not yet rated, you need to rate them as “1 star” in MM4. Tracks you want to exclude from computation need to have their ratings removed (zero stars). Archsimian will know the 1 star tracks are actually new/not-yet-rated tracks. Be careful because if you have existing tracks with 1 star (not because they are new, but because they were rated low) you will need to first change them (to zero if you want to exclude from playlist, or two stars to include).

Below are the Archsimian rating codes and associated MM4 star ratings:
 
<p>1 – new song not yet rated (one star)</p>
<p>(2 is reserved)</p>
<p>3 – five stars</p>
<p>4 – four stars</p>
<p>5 – three and one half stars</p>
<p>6 – three stars</p>
<p>7 – two and one half stars</p>
<p>8 – two stars</p>

Important – ArchSimian stores the above rating codes in the “Grouping” tag field” for all tracks in your library based on the star ratings you have. It is better to configure these rating codes yourself (although not required). This is important because the codes can be both stored and changed on the tag. You can then change a track rating from within Linux directly to the tag. This is easy to set up in MM4. First sort all tracks by their star rating. Select all the tracks of a particular rating, then right-click to select properties. Under the “Details” tab of the tag window, enter “3” to code your selected five-star tracks, “4” for four-star tracks, etc using the above key. Remember to code unrated tracks to Grouping “0” and one star (new tracks) to Grouping “1”.  The “star-rating” field is not used because it has been problematic for writing tag info into it, so the “Grouping” field is used here. You can reconcile rating changes made in Linux by creating smart playlists in MM4 to track mismatches between codes and star ratings (and update the star ratings in MM4 to match your rating code changes) if you change your tags in Linux using the rating codes.

<b>Exporting and Importing between MediaMonkey and ArchSimian</b>

To use ArchSimian, (unfortunately) you have to <i>manually</i> export from MediaMonkey the MM4 playlist you will be modifying in ArchSimian. As stated in the MM4 manual: 

To export specific .m3u files:
1.	Select the Tracks you wish to export from a Playlist
2.	From the File menu click Export to Playlist or right-click Send to .m3u Playlist
3.	Choose a folder and filename (xxxxx.m3u) for the Playlist

The Playlist will be saved as xxxxx.m3u, which can then be opened by another media player.

The default name MM4 gives you is “New playlist.m3u,” but you can save it as the name of the playlist you are modifying. After you revise the playlist in ArchSimian, it will place it in the folder where you placed the MM4 exported playlist. You can then import it back into MM4 using the ImportM3U plugin and update the playlist in MM4.

When you have added tracks from ArchSimian, you can export back to your playlist a Windows-compatible playlist that can be imported by MM4.

