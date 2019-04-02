# ArchSimian
You worked hard to build a large music library (2,000+ rated tracks), so make the most out of it with ArchSimian!
<img src="http://i.imgur.com/r7WxYoa.png..." data-canonical-src="http://i.imgur.com/r7WxYoa.png" width="502" height="600" />
<p>You may want this program if you have a large music library and want to build a more complex set of rules for populating a playlist. Here, you can have a more controlled selection of tracks, with <i>better variety</i>. This program considers not only the rating and last time played, but also the <b>specific order of tracks on the playlist</b>. It also considers custom artist groupings.</p>

For example, let's say you have two 'different' artists, "Tom Petty" and "Tom Petty & the Heartbreakers." This is basically the same artist and so you wish to treat them both as the same artist for purposes of when the artist should be repeated. You store a common name for both (eg. the custom grouping "Petty") in MM4 (using each tag's "Custom2" field), then, ArchSimian will treat either name as the same artist. With this in mind, you can create any custom artist groups you like, such as "Pop1971" with different but similar artists and you choose to treat them as one artist. It is up to you.

Users import a playlist from MM4, add tracks using ArchSimian, then export the improved playlist back into MM4 for use in Windows (and to sync with mobile devices using the MM4 for Android App). 

<b>Improving MediaMonkey 4 Gold by improving playlist creation</b>

Why would anyone want to do this, and how can Arch make MediaMonkey 4 Gold (MM4) better than it already is? 

It depends on whether you have a large music library, and whether you are happy with the smart playlist features in MM4. 

<b>What does ArchSimian do?</b>

<b>It creates refined playlists using existing data from your MediaMonkey database file.</b>

The playlist improvement concept is this: the play history in the MM database can be used to compute a user’s average listening time per day. By also calculating the number of rated tracks, along with the ratings and playing times for each, it can be determined how frequently a song should be added for a repeat play onto the playlist, and whether a specific artist should be added for the next track. While MM4 already has a smart playlist feature for “days since last played” by rating, it does not do so with a high degree of specialization for variety, such as the arrangement of tracks within the list (next artist added).

<b>(TL;DR) Quickstart:</b>

1. Rate all tracks in MediaMonkey you want to include in the ArchSimian-produced playlist. You should have 2,000 or more for it to work well. MediaMonkey should be "auto-organizing" your library and filenames <b> must</b> not have any spaces in them. Your music library dir tree must look like ->  Drive:/artist/album/song in Windows. If your tracks and directories are not formatted this way, ArchSimian <b>will not run</b>.

2.  A one-star rating is special in ArchSimian. Assign one-star in MediaMonkey for new tracks you have not yet rated, if you plan to enable this feature in ArchSimian. This tells ArchSimian to include new tracks in the selection process. You can set the frequency for new tracks separately. The tag fields in MediaMonkey for "Grouping" and "Custom2" are special to ArchSimian, so if you are currently using these fields, you may have to change them (if so, read details).

3. Assign zero stars in MediaMonkey to exclude tracks from the ArchSimian-produced playlist.

4. You can (and should) set 'custom' artist groupings in MediaMonkey using the "Custom2" field. ArchSimian will override artist tags wherever Custom2 is used. It is a best practice to spend some time with the artist groupings. If you make sure all 'variances' to artist names have been reconciled this way, you will get the most real variety in your playlist.

5. Export the MM.DB (safest is to use a backup) to a dir you will select in ArchSimian setup menu. Export playlists for use in ArchSimian using the "manual export" function found under MediaMonkey's "File" menu.

6. Install ArchSimian. You will get a starting menu for setting up locations for MM.DB backup, location of (shared) music library (select the top dir of your music library), and directory where playlists are stored. Restart the program. It will then process your MM.DB and launch with all of its features enabled. Play with the frequency tab settings to your liking (restart), then select a playlist and add some tracks and export the changes back to your playlist directory.

7. ArchSimian lets you set how tracks will be added to the playlist, and after adding tracks, it exports the modified playlist back to the dir where it was originally selected, overwriting the older playlist. You will then have a Windows-compatible playlist that can be imported into MediaMonkey (using the .mmip ImportM3U).

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

Also, you can enable the album-variety feature. When enabled, you will hear a track from a different album than the one last played of the artist, and the criteria for applying the feature can be set by the user.  

Replay intervals – you can set the planned time between repeats based on rating. By using ratios of repeat frequency with average listening time, the program can determine track should be added to the playlist. Variety takes precedence over repeat frequency.  If you set two-star tracks to repeat every three years, but your actual history is they have all been heard in the last two years, they will still be added based on the 'relative frequency' to that of the other tracks. To 'transition to longer times' set at a lower repeat frequency than what you desire (need three years, set to four years).

If you have enabled the 'include new tracks' feature, you may get artist repeats more frequently. Let's say you have set the repeat interval for new tracks at 10. This means that for every ten tracks added, one new song will be added. If there is more than one artist who has new tracks, then the last new artist selected will not be selected for the next new track. So, you could hear the same artist after 20 tracks if the setting is 10 and there is more than one artist with new tracks.

<b>Hardware and Software Requirements:</b>

If you are running KDE and the Windows VirtualMachine (VM) together, you will need 8GB of RAM to run smoothly. Less might work with a lighter DE (but no guarantees). If you do not use a VM for Windows, then 4GB is enough.

If you do use the VM, you need a CPU capable of running Windows 10 on an Oracle VirtualBox Virtual Machine (VM), meaning a 64-bit processor with hardware virtualization support enabled. See here for details: <html>https://forums.virtualbox.org/viewtopic.php?t=58072</html>

A Linux OS (I have tested it in Arch, for other distros, I have not tested, and highly recommend KDE as your Desktop Environment (DE), mainly for the notifications it gives you; also conky if you want your music player integrated onto the desktop itself). You may be able to make this work with other Linux distros or DEs, but your mileage will vary and any changes needed are not addressed here. 

MediaMonkey4 Gold for Windows 

Note: while it could installed on a separate partition of Windows using a dual boot system, it is far easier installed and running in a Windows 10 VM running in Arch. Even if you have Windows installed on a dual boot, you will want to also have the VM installed in Arch if you want to avoid having to reboot twice for every library/playlist update). 

Separately, you need to install/setup several scripts (.mmip files) in MM4:
 
Backup 6.0 (trixmoto) for backing up the MM4 database; 
Update Location of Files in Database (if you install a VM); 
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

Library - ensure the file hierarchy of your MM4 music library drive conforms to the hierarchy of [drive]/artist/album, with no additional subfolders. All music folders and files should be in lower case and have no spaces or special characters (filename example: 01_-_alexi_murdoch_-_towards_the_sun.mp3).  To do file and folder naming, first go to Tools > AutoOrganize Files, and select the radio button for “Move and rename files to new destination based on file tags.” This lets MM4 manage your directory structure. Then, under destination, where “X” is the drive letter assigned to your music library. Enter this line, which will ensure the filenames are lower case letters and they do not have spaces:

X:\$Replace($lower(<Artist>), ,_)\$Replace($lower(<Album>), ,_)\<Track#:2>_-_$Replace($lower(<Artist>), ,_)_-_$Replace($lower(<Title>), ,_)

More on AutoOrganize filename changing here: <html>https://www.mediamonkey.com/sw/webhelp/frame/index.html?configuringdirectoryandfileformats.htm</html>

Tags - the fields for "Grouping" and "Custom2" are special for ArchSimian. if you currently use them for some other purpose, it will cause issues unless they are either blank, or are used based on these instructions. Custom2 is for custom artist designation, and Grouping is used if you wish to to directly place rating codes specific to ArchSimian (not required, and if you do not directly place them, <b>make sure this field is blank for all tracks</b>.

<b>MM4 Installation in the VM</b> – If you use a Windows VM using Oracle VirtualBox in Linux, install MM4 Gold and enter your license verification, then install .mmip scripts for Backup 6.0, Update Location of Files in Database, and ImportM3U. To use your original MM.DB and ini files in the VM, locate them, back them up and (with VM version of MM4 closed) copy it to the VM location replacing the one created at install, then open MM4 in the VM and modify the file location pointers. To do that, you can use the script “Update Location of Files in Database.” If you set up guest additions in the VM, you should have the new drive number. Select the old and new paths, then check “Update paths in database only for files that exist at new location.” Do not copy files.

If you use the MM4 Android App to sync to your phone wirelessly, you can still use it while MM4 is running in the VM, but you may need to disable KDE from autoconnecting to your phone so the VM can recognize the device.

<b> Troubleshooting</b>

The program works well, but you will have problems if you do not configure your library correctly. If you have any issues, you should first consider this is the cause. Most common are spaces/special characters in the track filenames (see above), and special characters in the tags themselves. 

I have not yet written debugging code for this, but special characters within the music tags can be fixed in MM4. You can search in MM4 for tags containing special characters by using this search format:

 '/', '\', '?', ',', '|', ':', '.', '_', '(', ')', '[', ']', '&', '@', '#', '+', '*', '!', '-', ';' '”' 

Unfortunately, this method does <b>not</b> work for double quotation marks, which have to be changed, and you also will have to ensure no tags have carats (^) in them. So, the remedy is to install the .mmip “RegEx Find and Replace” (add on), then select the option for “Replace specified string with another one in <Into Field>...” You can then replace all instances of double quotation marks with single quotation marks for all tag fields (in particular you will find this occurrence in song titles and album titles) in your library. Carats need to be removed or replaced with a different character because it is the delimiter this program uses to extract the songs table from the MM.DB. If you have carats, the fields will not parse correctly in this program. It appears commas and single quotes do not cause any problem.

Again, ensure all tracks in MM4 you want to include in your playlists are rated prior to running ArchSimian. 

Tracks with no rating are excluded from playlist computation in ArchSimian, so for any new tracks not yet rated, you need to rate them as “1 star” in MM4. Tracks you want to exclude from computation need to have their ratings removed (zero stars). ArchSimian will know the 1 star tracks are actually new/not-yet-rated tracks. Be careful because if you have existing tracks with 1 star (not because they are new, but because they were rated low) you will need to first change them (to zero if you want to exclude from playlist, or two stars to include).

Below are the ArchSimian rating codes and associated MM4 star ratings:
 
<p>1 – new song not yet rated (one star)</p>
<p>(2 is reserved)</p>
<p>3 – five stars</p>
<p>4 – four stars</p>
<p>5 – three and one half stars</p>
<p>6 – three stars</p>
<p>7 – two and one half stars</p>
<p>8 – two stars</p>

ArchSimian uses the "star ratings" you have in MM4 and translates them into rating codes for its use. 

(If you wish, you can assign ArchSimian rating codes directly in MM4 using the “<b>Grouping</b>” tag field for all tracks in your library, according to the star ratings you have, but it is not required. Archsimian uses the star ratings and assigns the codes to its database. It does not modify your tags. You can change the tags yourself,though. First sort all tracks by their star rating. Select all the tracks of a particular rating, then right-click to select properties. Under the “Details” tab of the tag window, enter “3” to code your selected five-star tracks, “4” for four-star tracks, etc using the above key. Remember to code unrated tracks to Grouping “0” and one star (new tracks) to Grouping “1”.) 

<b> Initial Settings</b>

<p>The Settings tab has functions to set locations for needed files, and user can choose to include new tracks or enable album-level variety</p>
<img src="http://imgur.com/0f7CCTJ.png/..." data-canonical-src="http://imgur.com/0f7CCTJ.png" width="502" height="600" />

<b> Statistics</b>
<p>The Statistics tab lists detailed information about the composition of your music library</p>
<img src="http://imgur.com//CyUlYMx.png/..." data-canonical-src="http://imgur.com//CyUlYMx.png" width="502" height="600" />

<b> Frequency</b>
<p>The Frequency tab has functions to set the frequency for repeating tracks based on ratings. Factors are applied in a hierarchical fashion.</p>
<img src="http://imgur.com//znrCVa8.png/..." data-canonical-src="http://imgur.com//znrCVa8.png" width="502" height="600" />

<b> Albums</b>

<p>The Albums tab appears when the user selects "Album-level variety" in the Settings tab. The user can then set criteria for when album-level variety will be applied.</p>
<img src="http://imgur.com//nfRaqnE.png/..." data-canonical-src="http://imgur.com//nfRaqnE.png" width="502" height="600" />

<b>Exporting and Importing between MediaMonkey and ArchSimian</b>
To use ArchSimian, (unfortunately) you have to <i>manually</i> export from MediaMonkey the MM4 playlist you will be modifying in ArchSimian. As stated in the MM4 manual: 

<i>To export specific .m3u files:
1.	Select the Tracks you wish to export from a Playlist
2.	From the File menu click Export to Playlist or right-click Send to .m3u Playlist
3.	Choose a folder and filename (xxxxx.m3u) for the Playlist

The Playlist will be saved as xxxxx.m3u, which can then be opened by another media player.</i>

The default name MM4 gives you is “New playlist.m3u,” but you can save it as the name of the playlist you are modifying. After you revise the playlist in ArchSimian, it will place it in the folder where you placed the MM4 exported playlist. You can then import it back into MM4 using the ImportM3U plugin and update the playlist in MM4.

When you have added tracks from ArchSimian, you can export your playlist to the Windows shared directory as a Windows-compatible playlist. You can then import it back into MM4 for use in Windows and your mobile device if you have the MM app.

