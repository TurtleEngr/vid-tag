<div>
    <hr/>
</div>

# NAME vid-tag

Rename video files and set Title / Caption / Keywords metadata.

# SYNOPSIS

First run. Create config file: ./vid-tag.conf

    vid-tag -n -e "pEventId"

Edit config files, and/or use these options to override config file.

    vid-tag -e "pEventId" File1 [File2 ...]
        [-p "pInitials"] [-P "pName"] [-C "pCaption"] [-R "pCopyright"]
        [-E "pEventName"] [-c "pCity"] [-k "pKeyword"]
        [-f "pFilePattern"] [-t "pTitlePattern"] [-x "pExtra"]
        [-n] [-h] [-H pStyle] [-v] [-d]

# DESCRIPTION

vid-tag renames one or more video files and writes Title, Caption, and
Keywords metadata using **exiftool**.  The file's **Create Date** metadata
is read and used to date-stamp the output file name, the output title,
and the file-system mtime.

An example file, ./vid-tag-example.txt, will be created to show the
changes to the file with the current settings. This is best used with
the -n option and include all the files, so the time stamps will match
what is in the files.

Files are copied to the ./output/ directory, so original files are
not touched.

Files (in ./output/) will be renamed using fFilePattern and the exif
values will be added.

Large files, greater than 10 min, will be split into multiple 10 min
parts (because some hosting sites do not allow "large" files). The
exif time stamps for the split files will update for each 10 min part.

Defaults are read in this order (each layer overrides the previous):

- Built-in script defaults.
- Local config: **./vid-tag.conf** If the file does not exist it
will be created, and updated with the latest default settings.
- Command-line options will override all other settings.

After all the values are set, the results are saved to
**./vid-tag.conf** so the next run needs only needs the -e option and
the file list.

# OPTIONS

- **-e "pEventId"**

    Short event tag used in file names; spaces are converted to '\_'.
    Pattern token: **%e**.  Config key: `event.tag`. Required: "pEventId"

    The value will be changed to lower-case.

- **-c "pCity"**

    City name.  Pattern token: **%c**.  Config key: `event.city`. Default:
    "city name"

- **-C "pCaption"**

    Caption / description metadata.  Config key: `com.caption`. Default:
    "Copyright TERMS"

- **-R "pCopyright"**

    Copyright stamp. Config key: `com.copyright` Default: "Copyright NAME YYYY"

- **-E "pEventName"**

    Long event name used in the title.  Pattern token: **%E**.
    Config key: `event.title`. Default: "Long event description."

- **-f "pFilePattern"**

    Output file-name pattern.  Config key: `com.file-pat`. Default:
    `%d_%e_%p_%f`.

- **-k "pKeyword"**

    Comma-separated keyword list.  Config key: `event.keyword`. Default:
    "PBP, event"

- **-p "pInitials"**

    Photographer's initials / short name (no spaces).  Pattern token: **%p**.
    Config key: `com.name-short`.

- **-P "pName"**

    Photographer's full name.  Pattern token: **%P**.
    Config key: `com.name-long`. Default: First Last

- **-t "pTitlePattern"**

    Title pattern. Config key: `com.title-pat`. Default: `%D, %c, %E, by %P.`

- **-x "pExtra"**

    If non-empty, appended to the generated title.
    Config key: `event.extra`.

    When setting, include a leading space and an ending '.'

- **-n**

    No-execute / dry-run.  Validate inputs and create config file
    **./vid-tag.conf**, but do not create output/, rename files, or modify
    metadata.

- **-h**

    Output this "long" usage help.  Same as `-H long`.

- **-H pStyle**

    Output usage in a specific style:

        short|usage - short usage, text
        long|text   - full usage, text
        man         - man page
        html        - HTML page
        md          - Markdown

- **-v**

    Verbose output.  Repeat for more detail (**-vv**).

        (default) - warning and above
        -v        - notice and above
        -vv       - info and above

- **-V**

    Output the script's version.

- **-d**

    Debug level. Default: 0
    Each -d increments the debug level. Log messages will be print
    if the debug level is greater than the log message's debug level.

## Pattern Tokens

    %c   city name            (-c)
    %d   file date short      "YYYY-MM-DD"
    %D   file date long       "YYYY-MM-DD WWW"
    %T   file date full       "YYYY-MM-DD_HH:MM:SS"
    %e   event short          (-e)
    %E   event long           (-E)
    %f   original file name   (path stripped)
    %p   photographer short   (-p)
    %P   photographer long    (-P)
    %x   extra title          (-x)

## Config File Format

This is the config file (`./vid-tag.conf`) format.
The value of `$gpEventId` is the primary key.

    [vid-tag "com"]
        caption    = pCaption
        copyright  = pCopyright
        file-pat   = %d_%e_%p_%f (pFilePatern)
        name-long  = Photographer Name (pName)
        name-short = Initials (pInitial)
        title-pat  = %D, %c, %E, by %P. (pTitlePattern)
    [vid-tag "pEventId"]
        title      = Long Event Title (pEventName)
        extra      = Second line for title. (pExtra)
        city       = pCity
        keyword    = word, word (pKeyword)
        date       = YYYY-MM-DD_HH:MM:SS (date read from first file)

extra - is optional, but if defined, it must end with a '.'

## CONFIG KEY TO VARIABLE MAP

These are internal variables.

    Config key        Variable
    ---------         --------
    com.caption       gpCaption
    com.copyright     gpCopyright
    com.file-pat      gpFilePat
    com.name-long     gpNameLong
    com.name-short    gpNameShort
    com.title-pat     gpTitlePat
    event.tag         gpEventId
    event.city        gpEventCity
    event.extra       gpEventExtra
    event.file-list   gpEventFileList
    event.keyword     gpEventKeyword
    event.title       gpEventTitle

# RETURN VALUE

0 on success; non-zero on error.

# ERRORS

Fatal:

    + No input files given.
    + exiftool is not installed or not on PATH.
    + File not found: <filename>

Warnings (processing continues):

    + No CreateDate in <file>; using file mtime.
    + output/<file> already exists, skipping link.
    + target exists, skipping: <old> -> <new>
    + exiftool returned non-zero on <file>

# ENVIRONMENT

HOME, USER, Tmp, gpVerbose, gpDebug, gpConfLocal

# FILES

    ./vid-tag.conf - config file describing tags for an event
    ./vid-tag-example.txt - lists the changes to files
    ./output/      - generated directory, with renamed copies of files

    ./vid-tag      - this script
    ./vid-tag.inc  - vid-tag functions
    ./bash-com.inc - utility functions
    
    ./vid-tag.test - optional unit and CLI integration tests
    ./vid-tag-test-input.zip - only needed for testCli* tests
    ./shunit2.1    - unit test framework for vid-tag.test
    

# SEE ALSO

Required: bash, exiftool, git config, ffmpeg, sed

Optional: vid-tag.test

# NOTES

# AUTHOR

Turtle Engineer

# HISTORY

GPLv2 (c) Copyright

cVer=1.1
