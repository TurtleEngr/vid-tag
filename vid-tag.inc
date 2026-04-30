set -u

export cVer=1.1
export cOutputDir

declare -lx gpEventId
export gFile
export gFileDate
export gFileDateFull
export gFileDateLong
export gFileDateShort
export gFileDateTouch
export gpCaption
export gpConfLocal
export gpCopyright
export gpDebug
export gpEventCity
export gpEventExtra
export gpEventFileList
export gpEventKeyword
export gpEventTitle
export gpFilePat
export gpNameLong
export gpNameShort
export gpNoExec
export gpTitlePat

# Captured command line for the example file.  Set in vid-tag main.
export gpCmdLine

# Per-file collection arrays used by fExample.  Indexed by gExampleIdx.
declare -ag gExampleOrigNames
declare -ag gExampleNewNames
declare -ag gExampleTitles
declare -ag gExampleCreateDates
declare -ig gExampleIdx

# ========================================
# Script Functions

# --------------------------------
fUsage() {
    # Quick help, run this:
    # vid-tag -h
    local pStyle="$1"

    case $pStyle in
        short | usage | man | long | text | md)
            fComUsage -f $cBin/$cName.inc -s $pStyle
            ;;
        html)
            fComUsage -f $cBin/$cName.inc -s $pStyle -t "$cName Usage"
            ;;
        *)
            fComUsage -f $cBin/$cName.inc -s usage
            ;;
    esac
    fCleanUp

    # POD Syntax: https://perldoc.perl.org/perlpod
    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=pod

=for text ========================================

=for html <hr/>

=head1 NAME vid-tag

Rename video files and set Title / Caption / Keywords metadata.

=head1 SYNOPSIS

First run. Create config file: ./vid-tag.conf

    vid-tag -n -e "pEventId"

Edit config files, and/or use these options to override config file.

    vid-tag -e "pEventId" File1 [File2 ...]
        [-p "pInitials"] [-P "pName"] [-C "pCaption"] [-R "pCopyright"]
        [-E "pEventName"] [-c "pCity"] [-k "pKeyword"]
        [-f "pFilePattern"] [-t "pTitlePattern"] [-x "pExtra"]
        [-n] [-h] [-H pStyle] [-v] [-d]

=head1 DESCRIPTION

vid-tag renames one or more video files and writes Title, Caption, and
Keywords metadata using B<exiftool>.  The file's B<Create Date> metadata
is read and used to date-stamp the output file name, the output title,
and the file-system mtime.

Files are copied to the ./output/ directory, so original files are
not touched.

Files (in ./output/) will be renamed using fFilePattern and the exif
values will be added.

Large files, greater than 10 min, will be split into multiple 10 min
parts (because some hosting sites do not allow "large" files). The
exif time stamps for the split files will update for each 10 min part.

Defaults are read in this order (each layer overrides the previous):

=over 4

=item Built-in script defaults.

=item Local config: B<./vid-tag.conf> If the file does not exist it
will be created, and updated with the latest default settings.

=item Command-line options will override all other settings.

=back

After all the values are set, the results are saved to
B<./vid-tag.conf> so the next run needs only needs the -e option and
the file list.

=head1 OPTIONS

=over 4

=item B<-e "pEventId">

Short event tag used in file names; spaces are converted to '_'.
Pattern token: B<%e>.  Config key: C<event.tag>. Required: "pEventId"

The value will be changed to lower-case.

=item B<-c "pCity">

City name.  Pattern token: B<%c>.  Config key: C<event.city>. Default:
"city name"

=item B<-C "pCaption">

Caption / description metadata.  Config key: C<com.caption>. Default:
"Copyright TERMS"

=item B<-R "pCopyright">

Copyright stamp. Config key: C<com.copyright> Default: "Copyright NAME YYYY"

=item B<-E "pEventName">

Long event name used in the title.  Pattern token: B<%E>.
Config key: C<event.title>. Default: "Long event description."

=item B<-f "pFilePattern">

Output file-name pattern.  Config key: C<com.file-pat>. Default:
C<%d_%e_%p_%f>.

=item B<-k "pKeyword">

Comma-separated keyword list.  Config key: C<event.keyword>. Default:
"PBP, event"

=item B<-p "pInitials">

Photographer's initials / short name (no spaces).  Pattern token: B<%p>.
Config key: C<com.name-short>.

=item B<-P "pName">

Photographer's full name.  Pattern token: B<%P>.
Config key: C<com.name-long>. Default: First Last

=item B<-t "pTitlePattern">

Title pattern. Config key: C<com.title-pat>. Default: C<%D, %c, %E, by %P.>

=item B<-x "pExtra">

If non-empty, appended to the generated title.
Config key: C<event.extra>.

When setting, include a leading space and an ending '.'

=item B<-n>

No-execute / dry-run.  Validate inputs and create config file
B<./vid-tag.conf>, but do not create output/, rename files, or modify
metadata.

=item B<-h>

Output this "long" usage help.  Same as C<-H long>.

=item B<-H pStyle>

Output usage in a specific style:

    short|usage - short usage, text
    long|text   - full usage, text
    man         - man page
    html        - HTML page
    md          - Markdown

=item B<-v>

Verbose output.  Repeat for more detail (B<-vv>).

    (default) - warning and above
    -v        - notice and above
    -vv       - info and above

=item B<-V>

Output the script's version.

=item B<-d>

Debug level. Default: 0
Each -d increments the debug level. Log messages will be print
if the debug level is greater than the log message's debug level.

=back

=head2 Pattern Tokens

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

=head2 Config File Format

This is the config file (F<./vid-tag.conf>) format.
The value of C<$gpEventId> is the primary key.

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

=head2 CONFIG KEY TO VARIABLE MAP

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

=head1 RETURN VALUE

0 on success; non-zero on error.

=head1 ERRORS

Fatal:

    + No input files given.
    + exiftool is not installed or not on PATH.
    + File not found: <filename>

Warnings (processing continues):

    + No CreateDate in <file>; using file mtime.
    + output/<file> already exists, skipping link.
    + target exists, skipping: <old> -> <new>
    + exiftool returned non-zero on <file>

=head1 ENVIRONMENT

HOME, USER, Tmp, gpVerbose, gpDebug, gpConfLocal

=head1 FILES

    ./vid-tag.conf - config file describing tags for an event
    ./output/      - generated directory, with renamed copies of files

    ./vid-tag      - this script
    ./vid-tag.inc  - vid-tag functions
    ./bash-com.inc - utility functions
    
    ./vid-tag.test - optional unit and CLI integration tests
    ./vid-tag-test-input.zip - only needed for testCli* tests
    ./shunit2.1    - unit test framework for vid-tag.test
    
=head1 SEE ALSO

Required: bash, exiftool, git config, ffmpeg, sed

Optional: vid-tag.test

=head1 NOTES

=head1 AUTHOR

Turtle Engineer

=head1 HISTORY

GPLv2 (c) Copyright

cVer=1.1

=cut

EOF

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head1 vid-tag Internal Documentation

=internal-head3 fUsage pStyle

Select the type of help output. See -h and -H options.

=internal-cut

EOF

} # fUsage

# --------------------------------
fCleanUp() {
    fComCleanUp
    exit

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fCleanUp

Calls fComCleanUp to remove temp files and exit cleanly.

=internal-cut
EOF
} # fCleanUp

# --------------------------------
fSetGlobals() {
    fComSetGlobals

    # -------------------
    # Config file locations for fComGetConfig / fComSetConfig
    gpConfLocal="./vid-tag.conf"

    # -------------------
    # Output directory constant
    cOutputDir="output"

    # -------------------
    # Script-level runtime flags
    gpNoExec=${gpNoExec:-0}

    # -------------------
    # Config-backed variables – hard-coded defaults.
    # Overridden in order: global conf → local conf → command line.
    gpCaption=${gpCaption:-"Copyright TERMS"}
    gpCopyright=${gpCopyright:-"Copyright NAME YYYY"}
    gpEventCity=${gpEventCity:-"TBD"}
    gpEventExtra=${gpEventExtra:-""}
    gpEventFileList=${gpEventFileList:-""}
    gpEventKeyword=${gpEventKeyword:-"TBD"}
    gpEventId=${gpEventId:-""}
    gpEventTitle=${gpEventTitle:-"TBD"}
    gpFilePat=${gpFilePat:-'%d_%e_%p_%f'}
    gpNameLong=${gpNameLong:-"TBD"}
    gpNameShort=${gpNameShort:-"TBD"}
    gpTitlePat=${gpTitlePat:-'%D, %c, %E, by %P.'}

    # -------------------
    # Per-file globals populated by fReadFileDate / fConvertDate
    gFile=""
    gFileDate=""
    gFileDateFull=""
    gFileDateLong=""
    gFileDateShort=""
    gFileDateTouch=""

    # -------------------
    # fExample collection state
    gpCmdLine=${gpCmdLine:-""}
    gExampleOrigNames=()
    gExampleNewNames=()
    gExampleTitles=()
    gExampleCreateDates=()
    gExampleIdx=0

    # -------------------
    # Required and optional external programs
    fComCheckDeps "bash exiftool git ffmpeg sed" "cat"

    return

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fSetGlobals

Calls fComSetGlobals, then sets all script-specific globals.

Sets gpConfLocal to the standard vid-tag config paths.

Hard-coded defaults for every gpVar are applied here.  They are
overridden in order by: global conf, local conf, command-line options.

=internal-cut
EOF
} # fSetGlobals

# --------------------------------
fLoadConf() {
    # Read event-specific overrides from the local config file.
    # The section prefix is the current value of gpEventId, which has
    # already been set from the global conf (and possibly pre-scanned
    # from the command line via -e).
    local tId

    if [[ ! -f "$gpConfLocal" ]]; then
        fLog -p info -m "Local config not found: $gpConfLocal" -l $LINENO
        return 0
    fi

    if [[ "$gpEventId" = "tbd" || -z "$gpEventId" ]]; then
        fError -m "gpEventId not set; cannot read local config sections." -l $LINENO
    fi

    # event.tag is the canonical source for gpEventId in local conf
    tId=$(fComGetConfig -l -k "$cName.$gpEventId.title")
    if [[ -z "$tId" ]]; then
        fError -m "Event not found in $gpConfLocal" -l $LINENO
    fi

    gpCaption=$(      fComGetConfig -l -k "$cName.com.caption"    -d "$gpCaption")
    gpCopyright=$(    fComGetConfig -l -k "$cName.com.copyright"  -d "$gpCopyright")
    gpFilePat=$(      fComGetConfig -l -k "$cName.com.file-pat"   -d "$gpFilePat")
    gpNameLong=$(     fComGetConfig -l -k "$cName.com.name-long"  -d "$gpNameLong")
    gpNameShort=$(    fComGetConfig -l -k "$cName.com.name-short" -d "$gpNameShort")
    
    gpTitlePat=$(     fComGetConfig -l -k "$cName.com.title-pat" -d "$gpTitlePat")

    gpEventTitle=$(   fComGetConfig -l -k "$cName.$gpEventId.title"     -d "$gpEventTitle")
    gpEventExtra=$(   fComGetConfig -l -k "$cName.$gpEventId.extra"     -d "$gpEventExtra")
    gpEventCity=$(    fComGetConfig -l -k "$cName.$gpEventId.city"      -d "$gpEventCity")
    gpEventKeyword=$( fComGetConfig -l -k "$cName.$gpEventId.keyword"   -d "$gpEventKeyword")
    gpEventFileList=$(fComGetConfig -l -k "$cName.$gpEventId.file-list" -d "$gpEventFileList")

    gpEventId="${gpEventId// /_}"
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fLoadLocalConf

Reads the local config file (gpConfLocal, default ./vid-tag.conf) using
fComGetConfig -l.  The section prefix is the current gpEventId value.

If the file is absent, or gpEventId is not yet known, the function
returns silently with a warning.

=internal-cut
EOF
} # fLoadConf

# --------------------------------
fSaveConf() {
    # Write the current values to the local config file under the
    # [$gpEventId ...] section.

    if [[ -z "$gpEventId" ]]; then
        fError -m "Cannot save local config: gpEventId is not set." -l $LINENO
    fi

    fLog -p info -m "Saving local config: $gpConfLocal (tag=$gpEventId)" -l $LINENO
    fComSetConfig -l -k "$cName.com.caption"     -v "$gpCaption"
    fComSetConfig -l -k "$cName.com.copyright"   -v "$gpCopyright"
    fComSetConfig -l -k "$cName.com.file-pat"    -v "$gpFilePat"
    fComSetConfig -l -k "$cName.com.name-long"   -v "$gpNameLong"
    fComSetConfig -l -k "$cName.com.name-short"  -v "$gpNameShort"
    fComSetConfig -l -k "$cName.com.title-pat"   -v "$gpTitlePat"

    fComSetConfig -l -k "$cName.$gpEventId.title"     -v "$gpEventTitle"
    fComSetConfig -l -k "$cName.$gpEventId.city"      -v "$gpEventCity"
    fComSetConfig -l -k "$cName.$gpEventId.keyword"   -v "$gpEventKeyword"

    if [[ -n "$gpEventFileList" ]]; then
       fComSetConfig -l -k "$cName.$gpEventId.file-list" -v "$gpEventFileList"
    fi
    if [[ -n "$gpEventExtra" ]]; then
       fComSetConfig -l -k "$cName.$gpEventId.extra"     -v "$gpEventExtra"
    fi

    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fSaveLocalConf

Writes the current effective values of all gp* config variables to the
local config file (gpConfLocal) under the [$gpEventId ...] sections,
using fComSetConfig -l.

Called when -s is present on the command line.

=internal-cut
EOF
} # fSaveConf

# --------------------------------
fPreScanOpts() {
    # First-pass option scan: extract only -e (event tag)
    # before loading the local config file.  The full getopts parse in
    # Main will apply all options after the config files are loaded.

    local OPTIND=1
    local OPTARG
    
    while (( $# > 0 )); do
        case "$1" in
            -e)
                gpEventId="${2// /_}"
                return 0
                ;;
            -h)
                fUsage long
                return 0
                ;;
            -H)
                fUsage $2
                return 0
                ;;
            -V) return 0 ;;
            -*)
                # Unknown flag: skip it and its argument (if present)
                shift
                # If the next token isn't another flag, consume it too
                if [[ "${1-}" != -* && -n "${1-}" ]]; then
                    shift
                fi
                ;;
            *)
                shift
                ;;
        esac
    done

    if [[ -z "$gpEventId" ]]; then
        fError -m "Missing pEventId, see required -e option." -l $LINENO
    fi

    if [[ ! -f "$gpConfLocal" ]]; then
        fSaveLocalConf
    fi
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fPreScanOpts

Scans command-line arguments for -e and -s before the config files are
loaded.  This ensures that fLoadLocalConf uses the correct event tag
even when -e is supplied on the command line.

A full option parse is done later in Main after config loading.

=internal-cut
EOF
} # fPreScanOpts

# --------------------------------
fGetOpt() {
    local OPTIND=1
    local OPTARG
    local tArg

    while getopts :c:C:e:E:f:k:np:P:R:t:x:hH:vVd tArg; do
        case $tArg in
            # Script arguments
            c) gpEventCity="$OPTARG" ;;
            C) gpCaption="$OPTARG" ;;
            e) gpEventId="${OPTARG// /_}" ;;
            E) gpEventTitle="$OPTARG" ;;
            f) gpFilePat="$OPTARG" ;;
            k) gpEventKeyword="$OPTARG" ;;
            n) gpNoExec=1 ;;
            p) gpNameShort="$OPTARG" ;;
            P) gpNameLong="$OPTARG" ;;
            R) gpCopyright="$OPTARG" ;;
            t) gpTitlePat="$OPTARG" ;;
            x) gpEventExtra="$OPTARG" ;;
            # Common arguments
            h) fUsage long ;;
            H) fUsage "$OPTARG" ;;
            v) ((++gpVerbose)) ;;
            V) echo "vid-tag version: $cVer"
               exit 0
               ;;
            d) ((++gpDebug)) ;;
            # Problem arguments
            :)  fError -m "Value required for option: -$OPTARG" -l $LINENO ;;
            \?) fError -m "Unknown option: $OPTARG"             -l $LINENO ;;
        esac
    done
    ((--OPTIND))
    shift $OPTIND
    if [[ $# -ne 0 ]]; then
        gpEventFileList="$*"
    fi

    # Normalise event id one final time (may have been set via -e above).
    gpEventId="${gpEventId// /_}"
} # fGetOpt

# --------------------------------
fValidate() {
    local tFile

    if [[ ! -w ${gpConfLocal%/*} ]]; then
        fError -m "Directory ${gpConfLocal%/*} is not writable." -l $LINENO
    fi

    if [[ "${gpFilePat#*%}" = "${gpFilePat}" ]]; then
        fError -m "-t pFilePat is missing % options" -l $LINENO
    fi
    if [[ "${gpTitlePat#*%}" = "${gpTitlePat}" ]]; then
        fError -m "-t pTitlePat is missing % options" -l $LINENO
    fi

    if [[ $gpNoExec -ne 0 ]]; then
        # Don't check the files
        return 0
    fi

    if [[ -z "$gpEventFileList" ]]; then
        fError -m "No input files given. Use -h for help." -l $LINENO
    fi

    for tFile in $gpEventFileList; do
        if [[ ! -f "$tFile" ]]; then
            fError -m "File not found: $tFile" -l $LINENO
        fi
    done
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fValidate

Verify that at least one input file was given and that every file in
gpEventFileList exists on disk.  Calls fError (fatal) on the first
missing file.

=internal-cut
EOF
} # fValidate

# --------------------------------
fConvertDate() {
    # Given an exiftool date string "YYYY:MM:DD HH:MM:SS", populate the
    # four gFileDate* globals used by fApplyPattern and fProcessFile.
    local pRaw="$1"
    
    local tNorm

    # Normalise date separator: "YYYY:MM:DD HH:MM:SS" -> "YYYY-MM-DD HH:MM:SS"
    tNorm=$(echo "$pRaw" \
        | sed 's/^\([0-9]\{4\}\):\([0-9]\{2\}\):\([0-9]\{2\}\)/\1-\2-\3/')

    gFileDateShort=$( 'date' -d "$tNorm" +"%Y-%m-%d")
    gFileDateLong=$(  'date' -d "$tNorm" +"%Y-%m-%d %a")
    gFileDateFull=$(  'date' -d "$tNorm" +"%Y-%m-%d_%H:%M:%S")
    gFileDateTouch=$( 'date' -d "$tNorm" +"%Y%m%d%H%M.%S")
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fConvertDate "YYYY:MM:DD HH:MM:SS"

Converts the exiftool CreateDate string into four gFileDate* globals:

    gFileDateShort  - "YYYY-MM-DD"
    gFileDateLong   - "YYYY-MM-DD WWW"
    gFileDateFull   - "YYYY-MM-DD_HH:MM:SS"
    gFileDateTouch  - "YYYYMMDDHHmm.SS"  (touch -t format)

=internal-cut
EOF
} # fConvertDate

# --------------------------------
fReadFileDate() {
    # Read CreateDate via exiftool.  Fall back to file mtime if absent.
    local pFile="$1"

    gFileDate=$(exiftool -s -s -s -CreateDate "$pFile" 2>/dev/null)
    if [[ -z "$gFileDate" ]]; then
        fLog -p warning \
            -m "No CreateDate in $pFile; using file mtime." -l $LINENO
        gFileDate=$('date' -r "$pFile" +"%Y:%m:%d %H:%M:%S")
    fi
    fConvertDate "$gFileDate"
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fReadFileDate pFile

Read the CreateDate metadata from pFile using exiftool and call
fConvertDate to populate the gFileDate* globals.

If exiftool finds no CreateDate, the file's mtime is used as a
fallback and a warning is logged.

=internal-cut
EOF
} # fReadFileDate

# --------------------------------
fApplyPattern() {
    # Expand %-tokens in pPattern, substituting current date/event globals.
    # $1 = pattern string   $2 = original basename (for %f)
    local pPattern="$1"
    local pFile="$2"
    
    local tR="$pPattern"

    # Expand longer / upper-case tokens first where case-only pairs exist.
    tR="${tR//%D/$gFileDateLong}"
    tR="${tR//%d/$gFileDateShort}"
    tR="${tR//%T/$gFileDateFull}"
    tR="${tR//%E/$gpEventTitle}"
    tR="${tR//%e/$gpEventId}"
    tR="${tR//%P/$gpNameLong}"
    tR="${tR//%p/$gpNameShort}"
    tR="${tR//%c/$gpEventCity}"
    tR="${tR//%f/$pFile}"

    echo "$tR"
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fApplyPattern pPattern pFile

Expand %-tokens within pPattern using the current gFileDate* and gp*
globals.  The expanded string is echoed to stdout.

Token table:

    %c   gpEventCity
    %d   gFileDateShort  "YYYY-MM-DD"
    %D   gFileDateLong   "YYYY-MM-DD WWW"
    %T   gFileDateFull   "YYYY-MM-DD_HH:MM:SS"
    %e   gpEventId
    %E   gpEventTitle
    %f   pFile  (basename of the original file)
    %p   gpNameShort
    %P   gpNameLong

=internal-cut
EOF
} # fApplyPattern

# --------------------------------
fCreateOutputDir() {
    # Create ./output/ and hard-link every input file into it.
    local tFile tBase

    if [[ ! -d "$cOutputDir" ]]; then
        mkdir -p "$cOutputDir"
        fLog -p info -m "Created output dir: $cOutputDir" -l $LINENO
    fi

    for tFile in $gpEventFileList; do
        tBase=$(basename "$tFile")
        if [[ -e "$cOutputDir/$tBase" ]]; then
            fLog -p warning \
                -m "$cOutputDir/$tBase already exists, skipping." \
                -l $LINENO
            continue
        fi
        fLog -m "Copy: $tFile -> $cOutputDir/$tBase" -l $LINENO
        rsync -aP $tFile $cOutputDir/$tBase
    done
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fCreateOutputDir

Creates cOutputDir (default: ./output/) if absent, then hard-links each
file in gpEventFileList into it.

Skips (with a warning) any link target that already exists.

=internal-cut
EOF
} # fCreateOutputDir

# --------------------------------
fExampleCollect() {
    # Append one record to the gExample* collection arrays.
    # $1 = original basename
    # $2 = exiftool-format CreateDate "YYYY:MM:DD HH:MM:SS"
    # Reads gpFilePat / gpTitlePat / gpEventExtra and the gFileDate*
    # globals (already populated by fConvertDate).
    local pBase="$1"
    local pCreateDate="$2"

    local tNewName
    local tNewTitle

    tNewName=$(  fApplyPattern "$gpFilePat"  "$pBase")
    tNewTitle=$( fApplyPattern "$gpTitlePat" "$pBase")
    if [[ -n "$gpEventExtra" ]]; then
        tNewTitle="$tNewTitle $gpEventExtra."
    fi

    gExampleOrigNames[$gExampleIdx]="$pBase"
    gExampleNewNames[$gExampleIdx]="$tNewName"
    gExampleTitles[$gExampleIdx]="$tNewTitle"
    gExampleCreateDates[$gExampleIdx]="$pCreateDate"
    ((++gExampleIdx))
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fExampleCollect pBase pCreateDate

Append one record (original name, new name, title, CreateDate) to the
gExample* arrays and bump gExampleIdx.

The gFileDate* globals must already be populated for the file (so that
fApplyPattern produces the right %d / %D / %T expansions).

=internal-cut
EOF
} # fExampleCollect

# --------------------------------
fExample() {
    # Build vid-tag-example.txt showing what the next non-dry-run pass
    # would do: rename mappings into ./output/ and the exif tags that
    # will be set on each file.
    #
    # Called after fValidate / fSaveConf so all gp* config values are
    # final.  When gpNoExec is set, no real files have been validated,
    # so we use a single dummy entry: name "FILE.mp4" and CreateDate
    # set to "now".  Otherwise we walk gpEventFileList and read each
    # file's CreateDate via fReadFileDate.

    local cExampleFile="vid-tag-example.txt"
    local tBase
    local tCmd
    local tFile
    local -i tIdx

    # Reset the per-file collection arrays so repeated calls don't
    # accumulate stale entries.
    gExampleOrigNames=()
    gExampleNewNames=()
    gExampleTitles=()
    gExampleCreateDates=()
    gExampleIdx=0

    # ---- Collection phase ----
    if [[ $gpNoExec -ne 0 ]]; then
        # Dummy entry: FILE.mp4 with the current local date/time
        gFileDate=$('date' +"%Y:%m:%d %H:%M:%S")
        fConvertDate "$gFileDate"
        fExampleCollect "FILE.mp4" "$gFileDate"
    else
        for tFile in $gpEventFileList; do
            tBase=$(basename "$tFile")
            fReadFileDate "$tFile"
            fExampleCollect "$tBase" "$gFileDate"
        done
    fi

    # ---- Output phase ----
    # Reconstruct the command line if vid-tag main did not capture it.
    if [[ -n "$gpCmdLine" ]]; then
        tCmd="$gpCmdLine"
    else
        tCmd="$cName"
        [[ $gpNoExec -ne 0 ]] && tCmd+=" -n"
        tCmd+=" -e $gpEventId"
        [[ -n "$gpEventFileList" ]] && tCmd+=" $gpEventFileList"
    fi

    {
        echo "Command:"
        echo "    $tCmd"
        echo "Files:"
        for (( tIdx=0; tIdx < gExampleIdx; tIdx++ )); do
            echo "    ${gExampleOrigNames[$tIdx]} -> $cOutputDir/${gExampleNewNames[$tIdx]}"
        done
        echo "Exif Changes:"
        for (( tIdx=0; tIdx < gExampleIdx; tIdx++ )); do
            echo "${gExampleNewNames[$tIdx]}"
            echo "    Title = ${gExampleTitles[$tIdx]}"
            echo "    Author = $gpNameLong"
            echo "    Artist = $gpCopyright"
            echo "    Copyright = $gpCopyright"
            echo "    Keywords = $gpEventKeyword"
            echo "    Caption = $gpCaption"
            echo "    Description = (see Caption)"
            echo "    Image Description = (see Caption)"
            echo "    Create Date = ${gExampleCreateDates[$tIdx]}"
            echo "    Date/Time Original = (see Create Date)"
            echo "    Modify Date = (see Create Date)"
            echo "    Track Create Date = (see Create Date)"
            echo "    Track Modify Date = (see Create Date)"
            echo "    Media Create Date = (see Create Date)"
            echo "    Media Modify Date = (see Create Date)"
        done
    } > "$cExampleFile"

    fLog -p info -m "Created $cExampleFile" -l $LINENO
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fExample

Build B<vid-tag-example.txt> in the current directory.  The file shows:

=internal-over 4

=internal-item 1. The (re-constructed) command line.

=internal-item 2. A "Files:" block listing each input file and its
intended renamed path under cOutputDir.

=internal-item 3. An "Exif Changes:" block listing the metadata tags
that vid-tag will write to each renamed file.

=internal-back

When gpNoExec is set, a single dummy entry is generated using
B<FILE.mp4> as the input name and the current wall-clock time as the
CreateDate, so the user can preview their conf settings before
supplying real files.  Otherwise each file in gpEventFileList is
read via fReadFileDate to get its CreateDate.

The per-file fields are accumulated into the gExample* arrays via
fExampleCollect, indexed by gExampleIdx.  The collection is read in
the output phase to format the file.

=internal-cut
EOF
} # fExample

# --------------------------------
fAddMin() {
    # Add pMin minutes to an exiftool "Create Date" grep line and
    # echo the adjusted timestamp in exiftool format.
    local pRaw="$1"
    local -i pMin=$2

    local tDate
    local tTime
    local -i tH
    local -i tM
    local -i tS

    # Strip the label, keeping only "YYYY:MM:DD HH:MM:SS"
    tDate="${pRaw##*: }"

    # Split into date and time parts
    tTime="${tDate##* }"
    tDate="${tDate%% *}"

    IFS=':' read -r tH tM tS <<< "$tTime"
    IFS=' '

    # Remove any leading zeros for arithmetic
    tM=$((10#$tM))
    tH=$((10#$tH))

    ((tM += pMin))
    if [[ $tM -ge 60 ]]; then
        ((tH += tM / 60))
        ((tM %= 60))
    fi

    printf "%s %02d:%02d:%02d" "$tDate" "$tH" "$tM" "$tS"
    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fAddMin "pCreateDateLine" pMinutes

Add pMinutes to the timestamp found in a "Create Date" a grep'ed line
from exiftool.  The result is echoed in exiftool format "YYYY:MM:DD
HH:MM:SS".

pCreateDateLine is the raw grep output, e.g.:
    "Create Date                     : 2026:03:01 19:57:49"

The label prefix is stripped automatically.  Only minute-to-hour
overflow is handled; hour-to-day overflow is not expected for
10-minute video segments.

=internal-cut
EOF
} # fAddMin

# --------------------------------
fProcessFile() {
    # Rename + tag a single file.  Must be called with CWD == ./output/
    local pFile="$1"

    local -i tH
    local -i tM
    local -i tS
    local -i tIncMin
    local tBase
    local tCmd
    local tCreateDate
    local tDate
    local tFile
    local tLength
    local tMin
    local tNewName
    local tNewTitle
    local tPart
    local tRC
    local tTime

    tBase=$(basename "$pFile")

    if [[ ! -f "$tBase" ]]; then
        fLog -p warning \
            -m "$tBase not found in $cOutputDir; skipping." -l $LINENO
        return 0
    fi
    fLog -m "Processing: $tBase" -l $LINENO

    fReadFileDate "$tBase"

    # ----------
    # Get the file's new name and the Title, using the patterns
    tNewName=$(  fApplyPattern "$gpFilePat"  "$tBase")
    tNewTitle=$( fApplyPattern "$gpTitlePat" "$tBase")
    if [[ -n "$gpEventExtra" ]]; then
        tNewTitle="$tNewTitle $gpEventExtra."
    fi

    # ----------
    # Collision guard before rename.
    if [[ "$tBase" != "$tNewName" && -e "$tNewName" ]]; then
        fLog -p warning \
            -m "target exists, skipping: $tBase -> $tNewName" -l $LINENO
        return 0
    fi

    # ----------
    # Use exiftool to rename AND set all metadata fields.
    tCmd=(
        exiftool
        -overwrite_original
        -FileName="$tNewName"
        -Title="$tNewTitle"
        -Copyright="$gpCopyright"
        -Artist="$gpCopyright"
        -Author="$gpNameLong"
        -Keywords="$gpEventKeyword"
        -Caption="$gpCaption"
        -Description="$gpCaption"
        -ImageDescription="$gpCaption"
        "$tBase"
    )
    fLog -p info -m "Rename: $tBase -> $tNewName" -l $LINENO
    fLog -p info -m "Set new meta values" -l $LINENO
    fLog -p debug-1 -m "Cmd: ${tCmd[*]}" -l $LINENO
    "${tCmd[@]}" 2>&1 | while IFS= read -r tLine; do
        fLog -p info -m "$tLine" -l $LINENO
    done
    tRC=${PIPESTATUS[0]}

    if [[ $tRC -ne 0 ]]; then
        fLog -p warning \
            -m "exiftool returned $tRC on $tBase" -l $LINENO
    fi

    # ----------
    # Align filesystem mtime with the metadata's CreateDate.
    fLog -p info -m "touch -d \"${gFileDateFull//_/ }\" \"$tNewName\"" -l $LINENO
    touch -d "${gFileDateFull//_/ }" "$tNewName"

    # ----------
    # See if the file needs to be split. I.e. greater than 10 min
    tLength=$(ffmpeg -i $tNewName 2>&1 | grep -oP 'Duration: \K[\d:]+(?=\.)')
    IFS=':' read -r tH tM tS <<< "$tLength"
    IFS=' '
    ((tMin = $tH * 60 + $tM))
    if [[ $tMin -le 10 ]]; then
        return 0
    fi

    # ----------
    # Yes, split the file
    fLog -m "Split $tNewName into 10min parts" -l LINENO
    ffmpeg -i $tNewName -c copy -map 0 -segment_time 00:10:00 -f segment -reset_timestamps 1 ${tNewName%.*}-%02d.mp4 >/dev/null 2>&1

    fLog -p info -m "Copy the exif data from original" -l $LINENO
    tCreateDate=$(exiftool $tNewName | grep 'Create Date')
    for tFile in ${tNewName%.*}-*.mp4; do
        fLog -p info -m "exiftool -TagsFromFile $tNewName -all:all $tFile" -l $LINENO
        exiftool -TagsFromFile $tNewName -all:all $tFile >/dev/null 2>&1
        
        # Increment the times for the parts
        tPart=${tFile%.mp4}
        tPart=${tPart##*-}
        ((tIncMin=tPart * 10))
        tNewTime=$(fAddMin "$tCreateDate" $tIncMin)
        exiftool \
            -DateTimeOriginal="$tNewTime" \
            -CreateDate="$tNewTime" \
            -ModifyDate="$tNewTime" \
            -TrackCreateDate="$tNewTime" \
            -TrackModifyDate="$tNewTime" \
            -MediaCreateDate="$tNewTime" \
            -MediaModifyDate="$tNewTime" \
            $tFile

        # Now set the file's timestamp
        tDate=${tNewTime% *}
        tTime=${tNewTime#* }
        fLog -p info -m "touch -d \"${tDate//:/-} $tTime\" $tFile" -l $LINENO
        touch -d "${tDate//:/-} $tTime" $tFile
    done
    rm *_original

    return 0

    # shellcheck disable=SC2317
    cat <<EOF >/dev/null
=internal-pod

=internal-head3 fProcessFile pFile

Process a single video file that has already been hard-linked into
cOutputDir.  Must be called with the working directory set to cOutputDir.

Steps:

=internal-over 4

=internal-item 1. Read CreateDate from file (fReadFileDate).

=internal-item 2. Expand file-name and title patterns (fApplyPattern).

=internal-item 3. Set filesystem mtime to match CreateDate (touch).

=internal-item 4. Run a single exiftool call to rename and set all metadata.

=internal-back

Logs at "info" level for each step; "warning" if exiftool fails.

=internal-cut

EOF

} # fProcessFile
