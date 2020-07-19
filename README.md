# ArchSimian
You worked hard to build a large music library (2,000+ rated tracks), so make the most out of it with ArchSimian!
<img src="http://i.imgur.com/mWmPTts.png..." data-canonical-src="http://i.imgur.com/mWmPTts.png" width="502" height="600" />
<p>You may want this program if you have a large music library with ratings assigned, and want to build a more refined set of rules for automatically populating a playlist. Here, you can have a more controlled selection of tracks, with <i>better variety</i>. This program considers not only the rating and last time played, but also the <b>specific order of tracks on the playlist</b>. It also considers custom artist groupings, and lets you control how frequently new tracks are played.</p>

Artist groupings let you control artist name variations. Let's say you have tracks for two "different" artists: "Tom Petty" and "Tom Petty & the Heartbreakers." Both are basically the same artist, so why not treat them as the same artist for considering when that artist should be repeated? You can store a common name (eg. using the custom grouping of "Petty") in MM4 (this is each tag's "Custom2" field), then, ArchSimian will treat either name as the same artist. With this in mind, you can create any custom artist groups you like (even if they are <b>not</b> similar). You could choose any group of artists and have them treated as if they were one artist. It is up to you.

Users can import a playlist from MM4, or export a new one to MM4. Add tracks using ArchSimian, and export the improved playlist back into MM4 for use in Windows (and to sync with mobile devices using the MM4 for Android App). 

<b>Improving MediaMonkey 4 Gold by improving playlist creation</b>

Why would anyone want to do this, and how can Arch make MediaMonkey 4 Gold (MM4) better than it already is? 

It depends on whether you have a large music library, and whether you are happy with the smart playlist features in MM4. 

<b>What does ArchSimian do?</b>

<b>It creates refined playlists using existing data from your MediaMonkey database file.</b>

The playlist improvement concept is this: the play history in the MM database can be used to compute a user’s average listening time per day. By also calculating the number of rated tracks, along with the ratings and playing times for each, it can be determined how frequently a song should be added for a repeat play onto the playlist, and whether a specific artist should be added for the next track. While MM4 already has a smart playlist feature for “days since last played” by rating, it does not do so with a high degree of specialization for variety, such as the arrangement of tracks within the list (next artist added).

<b>(TL;DR) Quickstart:</b>

1. Rate all tracks in MediaMonkey 4 (MM4) you want to include in ArchSimian. You should have 2,000 or more, but it might work with less. This program was tested with 8000+ rated tracks. MediaMonkey should be "auto-organizing" your library and filenames <b> must</b> not have any spaces or special characters in them. Your music library directory tree <b>must</b> look like ->  <i>Drive:/artist/album/song</i> OR <i>Drive:/music/artist/album/song</i> in Windows. The library tree cannot reside at a lower directory level.

2.  You can include or exclude new (unrated) tracks from being added to your playlist. If you wish to include them, the MM4 one-star rating is special for how ArchSimian treats them. Assign one-star in MM4 for new tracks you have not yet rated. This identifies new tracks in the selection process. In ArchSimian, you can also set the frequency for inserting a new track (i.e. every 10 songs). The tag fields in MM4 for "Grouping" and "Custom2" are also special for how ArchSimian treats them, so if you are currently using these fields, you may have to change them (if so, read details).

3. Assign zero stars in MM4 to exclude tracks from being added to ArchSimian-produced playlists.

4. You can (and should, but is not required) set 'custom' artist groupings in MM4 using the "Custom2" field. ArchSimian will override artist tags wherever Custom2 is used. It is a best practice to spend some time with the artist groupings. If you make sure all 'variances' to artist names have been reconciled this way, you will get the most real variety in your playlist.

5. Export the MM.DB (safest is to use a backup) from within MM4 to a dir you will then select in the ArchSimian setup menu. Export playlists for use in ArchSimian using the "manual export" function found under MediaMonkey's "File" menu. Do not use a 'backup' version.

6. Install ArchSimian. You will get a starting menu for setting up locations for MM.DB backup, location of (shared) music library (select the top dir of your music library), directory where playlists are stored. and the Windows drive letter of your music library. Restart the program. It will then process your MM.DB and launch with all of its features enabled. If there is any problem, you will find a diagnostics log in your .local/share/archsimian directory. Play with the frequency tab settings to your liking (restart), then select or create a playlist and add some tracks. Saving changes or creating new playlists will generate m3u files with song paths compatible with Windows directory format to the location you specify.

7. You will then have a Windows-compatible playlist, which can be imported into MediaMonkey (using the .mmip ImportM3U) for use in Windows or to sync for a portable device. IMPORTANT: Before each time you run ArchSimian, be sure to FIRST open MM4 and (1) synchronize your playlist(s) from any devices like smart-phones and ipod type devices, and then (2) back up your MediaMonkey database so ArchSimian will get all of your current LastPlayed dates, new tracks added, and ratings changes.

<b>The primary goal is to improve the variety of artists within the playlist, while at the same time considering rating and lastplayed date.</b> 

    • Obtains music tag data and other data stored within the MM.DB file to create a new database for
    this application
    • Using the database created, and statistics compiled from it, uses the data as constraints to 
    determine ‘optimum availability’ then iteratively selects each single track added to the chosen 
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

For artists with many rated songs, the “availability” of that artist may be more frequent, but without this program, you might end up with a song from the same artist after only 10 tracks, instead of 60 or 150. So in ArchSimian, the spacing is set automatically based on the total number of rated tracks for each artist, and based on the ratings assigned to all of the tracks in our library. The spacing between artist repeats is a function of how many total artists with rated tracks are in your library. 

You can also enable an 'album-variety' feature. If enabled, you will hear a track from a different album than the one last played of the artist. you can specify minimum number of albums, minimum number of tracks per album.  

Replay intervals – you can adjust the 'global' time between repeats with different intervals for each track rating. By using ratios of repeat frequency with average listening time, the program can determine when or whether a given track should be added to the playlist. Variety takes precedence over repeat frequency. 

If you enable the 'include new tracks' feature, you will get artist repeats more frequently, but you can control the frequency. Let's say you have set the repeat interval for new tracks at 10. This means that for every ten tracks added, one new song will be added. One useful feature of the program is that if there is more than one <i>new</i> artist, then the last new artist selected will be excluded from selection of the next new track. So, in this example, you could hear the same artist after 20 tracks when the repeat interval is 10 and you have more than one artist with new tracks.

<b>Hardware and Software Requirements:</b>

A Linux OS (I have tested it in Arch and KDE as the Desktop Environment), a desktop environment, and 4GB of RAM. You may be able to make this work with other Linux distros and DEs, but your mileage may vary. ArchSimian is a Qt5 application, using C++14.

Windows 7 or higher

MediaMonkey4 Gold for Windows 

<b>MediaMonkey Addons:</b>

You need to install/setup several scripts (.mmip files) in MM4 if you do not already use them:
 
Backup 6.0 (trixmoto: <html>https://www.rik.onl/mediamonkey/search.php?all=scripts</html>) for backing up the MM4 database;

Export/CreatExport for Child Nodes: <html>http://solair.eunet.rs/~zvezdand/ExportM3UsForSubNodes.htm</html>;

ImportM3U: <html>https://www.mediamonkey.com/addons/browse/item/import-m3u/</html>;

If a Windows VM:
Update Location of Files in Database: <html>http://solair.eunet.rs/~zvezdand/UpdateLocationOfFiles.htm</html>;

<b>Windows Configurations (dual boot vs. virtual machine) (See note 2)</b>

<b>Dependencies:</b>

SQLite3, qt5-base, git

Linux applications you might like to use along with this program (See note 3)

<b>Preparation</b>

<b>1) Operating Systems</b> - If you have multiple drives (or are currently using MM4 on Windows on the same computer), decide whether you want Arch installed on the same box. If you do, the best practice is for the Arch drive to be on a separate hard drive from that of Windows (for security reasons). 

Your music library should contain at least <b> 2,000 rated tracks </b> and be located on a windows-formatted (NTFS) storage drive that will be shared by both Windows and Linux. Set Linux file permissions accordingly for read-write access to the music library location. Again, it might run on fewer rated tracks, but testing has only been done on an 8,000+ rated library, so your mileage may vary.

<b>2) MediaMonkey 4.x Settings</b>

This program requires prior preparation in MM4. Backup your MM4.DB and .ini files before you start. 

Word to the wise: If you have to change your original MM4 configuration to use this program, you will be “stuck” with any changes made to your library filename/directory structure as described below, so if you are using some other filenaming/directory approach, either be prepared to backup your whole music library with the original filenaming/directory configuration, or accept the alternative of doing the “MM4 Configuration” section below then doing a second backup of the db and ini files. Remember all of this is at your own risk!

Even if you choose to not use ArchSimian, the below configuration changes are useful for configuring your library to access via a Linux distribution. The below configuration worked for me:

MM4 Configuration – Under Tools > Options > Library, enable the following: Infer file properties from filenames, use Tags then inferred properties; Get album titles and track numbers for tags only; Scan file directories for artwork; Update file info from tags when rescanning files. Under  Tools > Options > Library > Tags & Playlists, enable ID3v2 tags; ASCII Always; Update tags when editing properties; enable ‘Use Unicode (UTF-8 encoding) for M3U playlists, Warn when duplicate files are added; for artwork, Save image to file folder, First image per album/series: folder.jpg.

Tools > Options > Library > Appearance > Ignore prefixes

Library - ensure the file hierarchy of your MM4 music library drive conforms to the hierarchy of [drive:]/artist/album OR [drive:]/music/artist/album, with no additional subfolders. All music folders and files should be in lower case and have no spaces or special characters (filename example: 01_-_alexi_murdoch_-_towards_the_sun.mp3).  To do file and folder naming, first go to Tools > AutoOrganize Files, and select the radio button for “Move and rename files to new destination based on file tags.” This lets MM4 manage your directory structure. Then, under destination, where “X” is the drive letter assigned to your music library and "music" is a top folder on the drive, if used. Do not place the path below the level as shown in the "music" example. Enter this line, which will ensure the filenames are lower case letters and they do not have spaces:

X:\music\$Replace($lower(<Artist>), ,_)\$Replace($lower(<Album>), ,_)\<Track#:2>_-_$Replace($lower(<Artist>), ,_)_-_$Replace($lower(<Title>), ,_)

More on AutoOrganize filename changing here: <html>https://www.mediamonkey.com/sw/webhelp/frame/index.html?configuringdirectoryandfileformats.htm</html>

If you do it correctly, your music directory should look like this:
<img src="http://i.imgur.com/ZnEguuK.jpg..." data-canonical-src="http://i.imgur.com/ZnEguuK.jpg" width="608" height="475" />
<p>
    

Tags - the fields for "Grouping" and "Custom2" are special for ArchSimian. <b>If you currently use them for some other purpose, it will cause issues unless they are either blank, or are used based on these instructions.</b> Custom2 is for custom artist designation, and Grouping is used if you wish to to directly place rating codes specific to ArchSimian (not required). If you do not directly use either of these two fields, ArchSimian will still work correctly, <b> but just make sure these two fields are blank for every track tag</b>.

<b> Troubleshooting</b>

After you run the first setup menu, ArchSimian runs a diagnostic routine. Review the log at <i>.local/share/archsimian/diagnosticslog.txt</i> for recommendations if you encounter problems.

<b> ArchSimian Configuration</b>

Ensure all tracks in MM4 you want to include in your playlists are rated prior to running ArchSimian. 

Tracks with no rating are excluded from playlist computation in ArchSimian, so for any new tracks not yet rated, you need to rate them as “1 star” in MM4. Tracks you want to exclude from computation need to have their ratings removed (zero stars). ArchSimian will know the 1 star tracks are actually new/not-yet-rated tracks. Be careful because if you have existing tracks with 1 star (not because they are new, but because they were rated low) you will need to first change them (to zero if you want to exclude from playlist, or two stars to include).

ArchSimian uses the "star ratings" you have in MM4 and translates them into rating codes for its use. (See Notes 1 and 4)

<b> Settings</b>
<p>The Settings tab has functions to set locations for needed files, and user can choose to include new tracks or enable album-level variety. The first time the program is run this tab only will appear. The 3 locations including windows drive letter (not shown in this photo) must be identified for the program to run.</p>
<img src="http://imgur.com/dwwhb1B.png/..." data-canonical-src="http://imgur.com/dwwhb1B.png" width="502" height="600" />

<b> MM4.DB Update</b>

When you update MM4 (for tracks played, new tracks, changed ratings) and backup its MM.DB file, ArchSimian will automatically detect the new data when you launch the program, and will display a dialog to inform you to wait for the database to update.

<b> Statistics</b>
<p>The Statistics tab lists detailed information about the composition of your music library</p>
<img src="http://imgur.com//XA1Ztz5.png/..." data-canonical-src="http://imgur.com//XA1Ztz5.png" width="502" height="600" />

<b> Frequency</b>
<p>The Frequency tab has functions to set the frequency for repeating tracks based on ratings. Factors are applied in a hierarchical fashion.</p>
<img src="http://imgur.com//duceyte.png/..." data-canonical-src="http://imgur.com//duceyte.png" width="502" height="600" />

<b> Albums</b>

<p>The Albums tab appears when the user selects "Album-level variety" in the Settings tab. The user can then set criteria for when album-level variety will be applied.</p>
<img src="http://imgur.com//EMswcA4.png/..." data-canonical-src="http://imgur.com//EMswcA4.png" width="502" height="600" />

<b>Creating a New Playlist</b>

Select new file using the toolbar or menu, and tracks can be added. ArchSimian uses your MM4 listening history.

<b>Exporting and Importing between MediaMonkey and ArchSimian</b>
To use ArchSimian, you have to export from MediaMonkey the MM4 playlist you will be modifying in ArchSimian. The easy way is to install the "Export/Create Playlist for Child Nodes" extension in MM4. Configure to export as "ANSI" and set directory for using in ArchSimian. Then. simply right click on the playlist and export. If you don't want to use the extension, this is also a base feature of MM4, but you will have to do it more manually (See note 5). After you revise the playlist in ArchSimian, it will place it in the folder where you placed the MM4 exported playlist. You can then import it back into MM4 using the ImportM3U plugin and update the playlist in MM4.

When you have added tracks from ArchSimian, you can simply save your playlist to the Windows shared directory as a Windows-compatible playlist. You can then import it back into MM4 for use in Windows and your mobile device if you have the MM app.

<b>NOTES</b>
1. <b>Below shows the ArchSimian rating code assigned to each MM4 star rating:</b>
 
<p>1 – new song not yet rated (one star)</p>
<p>(2 is reserved)</p>
<p>3 – five stars</p>
<p>4 – four stars, and four and one half stars (ArchSimian treats them as a single rating)</p>
<p>5 – three and one half stars</p>
<p>6 – three stars</p>
<p>7 – two and one half stars</p>
<p>8 – two stars</p>

2. <b>Windows Configurations (dual boot vs. virtual machine):</b>

While Linux can be installed on a separate partition of Windows using a dual boot system, it is far easier to run a Windows 10 VM running within Linux. Even if you have Windows installed on a dual boot, you might also want a VM installed in Linux if you want to avoid having to reboot twice for every library/playlist update. 

If you are running KDE and the Windows VirtualMachine (VM) together, you will need 8GB of RAM to run smoothly (16GB is better!). Less might work with a lighter DE (but no guarantees). If you do not use a VM for Windows, then 4GB is enough.

If you do use the VM, you need a CPU capable of running Windows 10 on an Oracle VirtualBox Virtual Machine (VM), meaning a 64-bit processor with hardware virtualization support enabled. See here for details: <html>https://forums.virtualbox.org/viewtopic.php?t=58072</html> IMPORTANT: When you specify in VM settings the shared folder(s), deselect automount and specify the mount point with the drive letter of you choice.

MM4 Installation in the VM – If you use a Windows VM using Oracle VirtualBox in Linux, install MM4 Gold and enter your license verification, then install .mmip scripts for Backup 6.0, Update Location of Files in Database, and ImportM3U. To use your original MM.DB and ini files in the VM, locate them, back them up and (with VM version of MM4 closed) copy it to the VM location replacing the one created at install, then open MM4 in the VM and modify the file location pointers. To do that, you can use the script “Update Location of Files in Database.” If you set up guest additions in the VM, you should have the new drive number. Select the old and new paths, then check “Update paths in database only for files that exist at new location.” Do not copy files.

If you use the MM4 Android App to sync to your phone wirelessly, you can still use it while MM4 is running in the VM, but you may need to disable KDE from autoconnecting to your phone so the VM can recognize the device.

When installing the VM you have to add guest additions before you install MM4 so that you can configure access to the music library and the folder(s) you will use to store the MM4 backups for its database and for your playlists.

3. <b>Linux applications you might like to use along with this program:</b>

audacious-qt5 (linux media player that sounds great, and best of all integrates with conky perfectly). Install audacious-plugins-qt5 and apulse.

conky (if you wish to display cover art and most other tag elements right on a panel docked onto your desktop (cool!)

id3ted and kid3-cli for tag scripting requirements (if you want the ability to update tags in Linux).

Oracle VirtualBox VM (if you wish to run MM4 in Linux by running Windows 10 on a VM)

4. <b>If you wish, you can assign ArchSimian rating codes directly in MM4:</b> 

Use the “<b>Grouping</b>” tag field for all tracks in your library, according to the star ratings you have, but it is not required. ArchSimian uses the star ratings and assigns the codes to its database. It does not modify your tags. You can change the tags yourself, though. You may wish to do it this way if you do not want to change your star ratings in MM4. This lets you force Archsimian to read its codes directly while ignoring the star ratings you are using. 

First, sort all tracks by their star rating. Select all the tracks of a particular rating, then right-click to select properties. Under the “Details” tab of the tag window, enter “3” to code your selected five-star tracks, “4” for four-star tracks, etc., using the table above. Remember to code unrated tracks to Grouping “0” and one star (new tracks) to Grouping “1”.

5. <b>As stated in the MM4 manual:</b> 

<i>To export specific .m3u files:
1.	Select the Tracks you wish to export from a Playlist
2.	From the File menu click Export to Playlist or right-click Send to .m3u Playlist
3.	Choose a folder and filename (xxxxx.m3u) for the Playlist

The Playlist will be saved as xxxxx.m3u, which can then be opened by another media player.</i>

The default name MM4 gives you is “New playlist.m3u,” but you can save it as the name of the playlist you are modifying.
