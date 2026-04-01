cat << EOF > "$archname"
#!/bin/sh
# This script was generated using Makeself $MS_VERSION
# The license covering this archive and its contents, if any, is wholly
# independent of the Makeself license (GPL)


ARCH=x86_64
ARCHIVE_DIR="\${0%/*}"
ARCHIVE_NAME="\${0##*/}"
ORIG_UMASK=\$(umask)
SHA="$SHAsum"
SIGNATURE="$Signature"
TMPROOT=\${TMPDIR:=/tmp}
USER_PWD="\$PWD"
VER=26.04


debug="\$MUNADI_DEBUG"
export_conf="$EXPORT_CONF"
extractdir="$archdirname"
filesizes="$filesizes"
keep="$KEEP"
label="$LABEL"
nodiskspace="n"
nooverwrite="$NOOVERWRITE"
quiet="n"
script="$SCRIPT"
scriptargs="-s $SCRIPTARGS"
skip="$SKIP"
totalsize="$totalsize"

export ARCHIVE_DIR ARCHIVE_NAME USER_PWD
umask 022

print_cmd_arg=""
if hash printf 2>/dev/null; then
  print_cmd="printf"
elif hash print 2>/dev/null; then
  print_cmd="print"
else
  print_cmd="echo"
fi

if test -d /usr/xpg4/bin; then
  PATH=/usr/xpg4/bin:\$PATH
  export PATH
fi

if test -d /usr/sfw/bin; then
  PATH=\$PATH:/usr/sfw/bin
  export PATH
fi

unset CDPATH

MS_Printf()
{
  \$print_cmd \$print_cmd_arg "\$1"
}

MS_diskspace()
{
  (df -k "\$1" | tail -1 | awk '{ if (\$4 ~ /%/) {print \$3} else {print \$4} }')
}

MS_dd()
{
  blocks=\$((\$3 / 1024))
  bytes=\$((\$3 % 1024))
  # Test for ibs, obs and conv feature
  if dd if=/dev/zero of=/dev/null \
      count=1 ibs=512 obs=512 conv=sync 2> /dev/null; then
    dd if="\$1" ibs=\$2 skip=1 obs=1024 conv=sync 2> /dev/null | { \\
      test \$blocks -gt 0 && dd ibs=1024 obs=1024 iflag=fullblock count=\$blocks
      test \$bytes  -gt 0 && dd ibs=1 obs=1024 count=\$bytes ; } 2> /dev/null
  else
    dd if="\$1" bs=\$2 skip=1 iflag=fullblock 2> /dev/null
  fi
}

MS_dd_Progress()
{
  if test "\$noprogress" = y; then
    MS_dd "\$@"
    return \$?
  fi
  file="\$1"
  offset=\$2
  length=\$3
  pos=0
  bsize=4194304
  while test \$bsize -gt \$length; do bsize=\$((bsize / 4)); done
  blocks=\$((length / bsize))
  bytes=\$((length % bsize))
  (
    dd ibs=\$offset skip=1 count=1 2>/dev/null
    pos=\$((pos + bsize))
    MS_Printf "     0%% " 1>&2
    if test \$blocks -gt 0; then
      while test \$pos -le \$length; do
        dd bs=\$bsize count=1 2>/dev/null
        pcent=\$((length / 100))
        pcent=\$((pos / pcent))
        if test \$pcent -lt 100; then
          MS_Printf "\b\b\b\b\b\b\b" 1>&2
          if test \$pcent -lt 10; then
            MS_Printf "    \$pcent%% " 1>&2
          else
            MS_Printf "   \$pcent%% " 1>&2
          fi
        fi
        pos=\$((pos + bsize))
      done
    fi
    if test \$bytes -gt 0; then dd bs=\$bytes count=1 2>/dev/null; fi
    MS_Printf "\b\b\b\b\b\b\b" 1>&2
    MS_Printf " 100%%  " 1>&2
  ) < "\$file"
}

MS_Version_show()
{
  cat << EOH >&2
 Munadi - Simple athan app.
 version: \$VER (static; \$ARCH)
 List of linked libraries:
    libb2.so.1          libblkid.so.1       libbrotlicommon.so.1
    libbrotlidec.so.1   libbz2.so.1         libcom_err.so.2
    libcrypt.so.2       libcurl.so.4        libdouble-conversion.so.3
    libduktape.so.207   libevent-2.1.so.7   libfontconfig.so.1
    libfreetype.so.6    libgio-2.0.so.0     libglib-2.0.so.0
    libGLX.so.0         libgmodule-2.0.so.0 libgobject-2.0.so.0
    libgomp.so.1        libgraphite2.so.3   libgssapi_krb5.so.2
    libharfbuzz.so.0    libicudata.so.77    libicui18n.so.77
    libicuuc.so.77      libidn2.so.0        libk5crypto.so.3
    libkeyutils.so.1    libkrb5.so.3        libkrb5support.so.0
    liblber.so.2        libldap.so.2        liblzma.so.5
    libmount.so.1       libnghttp2.so.14    libpcre2-16.so.0
    libpcre2-8.so.0     libpng16.so.16      libproxy.so.1
    libpsl.so.5         libpxbackend-1.0.so libQt6Core.so.6
    libQt6DBus.so.6     libQt6Gui.so.6      libQt6Network.so.6
    libQt6Qml.so.6      libQt6Widgets.so.6  libresolv.so.2
    libsasl2.so.3       libselinux.so.1     libssh.so.4
    libunistring.so.5   libxml2.so.2        libzstd.so.1

EOH
}

MS_Help()
{
  _script="\$ARCHIVE_NAME"
  cat << EOH >&2

 Munadi \$VER installer, copyright munadi.org.

 Makeself version $MS_VERSION

 Help or info:
  --help, -h        Print this message.
  --info, -V,       Print various info : version, title, default target directory,
  --version         embedded script ...

  --list, -l        Print the list of files in the archive
  --check, -c       Checks integrity of the archive
  --verify-sig key  Verify signature against a provided key id
  --show-file file  Print the file named "file" in archive. The path of file
                    is relative to archive root.

 Archive Executing:
  \$_script [-h] [-V] [-l [-v]] [-c [-q]] [-q | -y] [-q | -v] [-k | -x dir]
    [--chown | --nochown] [options] [install | [ uninstall | remove ] ]

  Mandatory arguments for applicable to both short and long options.
  Invoking this file without any options installs at the default directory.

  --assumeyes, -y       Assume yes and do not ask
  --quiet, -q           Do not print anything except error messages (implies '-y')
  --exdir dir, -x dir   Extract directly to a extract directory (absolute or relative)
                        This directory may undergo recursive chown (see --nochown).
  --chown               Give the target folder to the current user recursively
  --nochown             Do not give the target folder to the current user
  --keep, -k            Do not erase extract directory after running
                        the embedded script
  --nodiskspace         Do not check for available disk space
  --noexec, -n          Do not run embedded script
  --noprogress          Do not show the progress during the decompression
  --tar arg1 [arg2 ...] Access the contents of the archive through the tar command
  --verbose, -v         Show more messages. '-q' will have no effect

  ENVIRONMENT
    MUNADI_DEBUG
      If set to 1, print debug messages from executing scripts.

    SETUP_NOCHECK
      If set to 1, then checksum validation will be skipped.
EOH
}

MS_Verify_Sig()
{
  GPG_PATH=\$(exec <&- 2>&-; which gpg || command -v gpg || type gpg)
  MKTEMP_PATH=\$(exec <&- 2>&-; which mktemp || command -v mktemp || type mktemp)
  test -x "\$GPG_PATH" || \\
    GPG_PATH=\$(exec <&- 2>&-; which gpg || command -v gpg || type gpg)
  test -x "\$MKTEMP_PATH" || \\
    MKTEMP_PATH=\$(exec <&- 2>&-; which mktemp || \\
      command -v mktemp || type mktemp)
  offset=\$(head -n "\$skip" "\$1" | wc -c | sed "s/ //g")
  temp_sig=\$(mktemp -t XXXXX)
  echo \$SIGNATURE | base64 --decode > "\$temp_sig"
  gpg_output=\$(MS_dd "\$1" \$offset \$totalsize | \\
    LC_ALL=C "\$GPG_PATH" --verify "\$temp_sig" - 2>&1)
  gpg_res=\$?
  rm -f "\$temp_sig"
  if test "\$gpg_res" -eq 0 && \\
      test "\$(echo \$gpg_output | grep -c Good)" -eq 1; then
    if test "\$(echo \$gpg_output | grep -c \$sig_key)" -eq 1; then
      test "\$quiet" = n && echo "GPG signature is good" >&2
    else
      echo "GPG Signature key does not match" >&2
      exit 2
    fi
  else
    test "\$quiet" = n && echo "GPG signature failed to verify" >&2
    exit 2
  fi
}

MS_Check()
{
  OLD_PATH="\$PATH"
  PATH=\${GUESS_MD5_PATH:-"\$OLD_PATH:/bin:/usr/bin:/sbin:/usr/local/ssl/bin:/usr/local/bin:/opt/openssl/bin"}
  PATH="\$OLD_PATH"

  SHA_PATH=\$(exec <&- 2>&-; which shasum || command -v shasum || type shasum)
  test -x "\$SHA_PATH" || SHA_PATH=\$(exec <&- 2>&-; which sha256sum || \\
    command -v sha256sum || type sha256sum)

  if test "\$quiet" = n; then MS_Printf "Verifying archive integrity..."; fi
  offset=\$(head -n "\$skip" "\$1" | wc -c | sed "s/ //g")
  fsize=\$(cat "\$1" | wc -c | sed "s/ //g")
  if test \$totalsize -ne \$((fsize - offset)); then
    echo " Unexpected archive size." >&2
    exit 2
  fi
  verb=\$2
  i=1
  for s in \$filesizes; do
    if test -x "\$SHA_PATH"; then
      if test "\${SHA_PATH##*/}" = shasum; then
        SHA_ARG="-a 256"
      fi
      sha=\$(echo \$SHA | cut -d" " -f\$i)
      if test "\$sha" = 0000000000000000000000000000000000000000000000000000000000000000; then
        test "\$verb" = y && echo " \$1 does not contain an embedded SHA256 checksum." >&2
      else
        shasum=\$(MS_dd_Progress "\$1" \$offset \$s | eval "\$SHA_PATH \$SHA_ARG" | cut -b-64);
        if test "\$shasum" != "\$sha"; then
          echo "Error in SHA256 checksums: \$shasum is different from \$sha" >&2
          exit 2
        elif test "\$quiet" = n; then
          echo "SHA256 checksums are OK."
        fi
        verb=n;
      fi
    fi
    i=\$((i + 1))
    offset=\$((offset + s))
  done
  if test "\$quiet" = n; then echo "### All good. >>>"; fi
}

MS_Preextract() {
  _errout="\$(mktemp "\$tmpdir/munadi.makeself.pre-extract.XXXXXX")"
  _logfile="\$(mktemp munadi.makeself.pre-extract-XXXXXX.log)" # Log to current
                                                               # dir
  _sane=1         # psuedo-bool
  _syslibs=''

  log() {
      MS_Printf "\$(date -Ins):munadi:pre-extract:\$*\n" >> "\$_logfile"
  }

  debug_print()
  {
    if [ "\$debug" = 1 ]; then
      MS_Printf "\\e[0;34mmunadi:pre-extract:debug::\\t\$1\e[0;m\\n" >&2
      log "debug:: \$1"
    fi
  }

  error_print()
  {
    if [ "\$quiet" = n ]; then
      MS_Printf "\\e[0;31mmunadi:pre-extract:error::\\t\$1\e[0;m\\n" >&2
      log "error:: \$1"
    fi
  }

  warn_print()
  {
    if [ "\$quiet" = n ]; then
      MS_Printf "\\e[0;33mmunadi:pre-extract:warn::\\t\$1\\e[0;m\\n" >&2
      log "warn:: \$1"
    fi
  }

  info_print()
  {
    if [ "\$verbose" = y ]; then
      MS_Printf "munadi:pre-extract:info::\\t\$1\\n"
      log "info:: \$1"
    fi
  }

  #+++++++++++++++++++++++++++++++++++++++++++++++++++
  # Know what EUID and EGID we are. We don't accept normal users having system GID

  _euid=\$(id -u)
  _egid=\$(id -g)
  _nuid=\$(id -un)
  _ngid=\$(id -gn)

  debug_print 'Checking UID and GID...'
  debug_print "Effective UID is \$_euid (\$_nuid)"
  debug_print "Effective GID is \$_egid (\$_ngid)"
  if [ "\$_euid" = 0 ]; then
    info_print 'Script is running with superuser privilege.'
    _priv=1
  elif [ "\$_euid" -ge 1000 ] && { \\
    [ \$_egid -ge 1000 ] || [ "\$_ngid" = "users" ]; }; then
    info_print 'Script is running with normal privilege.'
    _priv=0
  else
    error_print 'Unusual effective IDs. Cowardly refusing to proceed.'
    _sane=0
  fi
  #---------------------------------------------------

  #+++++++++++++++++++++++++++++++++++++++++++++++++++
  # Test if required programs are available on the system
  debug_print 'Checking if required programs are available...'
  if hash bash chmod date find install pkill rm rmdir sed uname \\
    xdg-desktop-menu xdg-icon-resource zstd 2>"\$_errout"; then
    debug_print "\\n\$(hash)"
  else
    _sane=0; error_print "\\n\$(cat "\$_errout")"
  fi

  if hash dbus-send 2>/dev/null; then
    if dbus-send --system --print-reply=literal \\
        --dest=org.freedesktop.DBus \\
        / org.freedesktop.DBus.ListNames > "\$_errout" 2>&1; then
      debug_print 'DBus system bus is operational.'
    else
      warn_print "No reply received. DBus system bus may not be operational."
      debug_print "\$(cat "\$_errout")"
    fi

    if _out="\$(dbus-send --session --dest=org.freedesktop.DBus \\
      --type=method_call --print-reply=literal /org/freedesktop/DBus \\
      org.freedesktop.DBus.NameHasOwner string:'org.kde.StatusNotifierWatcher'\\
      2>"\$_errout")"
    then
      debug_print 'DBus sesssion bus is operational.'
      if [ "\${_out##* }" = true ]; then
        debug_print "org.kde.KStatusNotifierWatcher found."
      else
        warn_print "org.kde.KStatusNotifierWatcher DBus service not found."
        warn_print "Tray icon won't work."
      fi
    else
      warn_print 'No reply received. DBus session bus may not be operational.'
      debug_print "\$(cat "\$_errout")"
    fi
  else
    warn_print '"dbus-send" not found, DBus may not be available.'
    warn_print "Some app features may not work."
  fi
  #---------------------------------------------------

  #+++++++++++++++++++++++++++++++++++++++++++++++++++
  # Ensure we have any form of wayland to display
  debug_print 'Checking if we have any form of wayland to display...'
  if [ -z "\$WAYLAND_DISPLAY" ] && [ "\${XDG_SESSION_TYPE:-}" != "wayland" ]
  then
    error_print 'Wayland display not found.'
    debug_print "WAYLAND_DISPLAY=\$WAYLAND_DISPLAY"
    debug_print "XDG_SESSION_TYPE=\$XDG_SESSION_TYPE"
    if [ "\$_priv" = 0 ]; then
      _sane=0
    else
  # We continue because the script may be running under 'sudo', 'su' or chroot
  # all for which the variables aren't usually available.
      error_print 'Continuing through previous error...'
    fi
  else
    debug_print "Wayland display socket found: \$WAYLAND_DISPLAY"
    debug_print "XDG_SESSION_TYPE is \$XDG_SESSION_TYPE"
  fi
  #---------------------------------------------------

  #+++++++++++++++++++++++++++++++++++++++++++++++++++
  # Check if required dynamic libraries are available on the system
  debug_print 'Checking required dynamic libraries...'
  if hash ldconfig 2>/dev/null; then
    debug_print 'Using ldconfig to search libraries'
    _syslibs="\$(ldconfig -p 2>/dev/null | sed -n 's/.* => //p')"
  else
    debug_print 'Searching required libraries in known paths...'
  fi

  # We currently check for lib basename, although older lib versions found may
  # potentially raise unexpected bugs.
  for lib in \\
    libEGL.so             libFLAC.so            libGLdispatch.so \\
    libOpenGL.so          libX11.so             libXau.so \\
    libXext.so            libXrandr.so          libXrender.so \\
    libasyncns.so         libc.so               libcap.so \\
    libcrypto.so          libdbus-1.so          libffi.so \\
    libgcc_s.so           libm.so               libmp3lame.so \\
    libmpg123.so          libogg.so             libopus.so \\
    libpulse.so           libsndfile.so         libssl.so \\
    libstdc++.so          libvorbis.so          libvorbisenc.so \\
    libwayland-client.so  libwayland-cursor.so  libwayland-egl.so \\
    libxcb.so             libxkbcommon.so       libz.so
  do
    if [ -z "\$_syslibs" ]; then
    _out="\$(find -H /lib /lib64 /usr/lib /usr/lib64 \\
        \\( -type l -o -type f \\) -iname "\$lib*" -print -quit 2>/dev/null)" ||:
      [ -n "\$_out" ] && { debug_print "\$lib found at \$_out"; continue; }
    else
      case " \$_syslibs " in
        *"\$lib"*) debug_print "\$lib found"; continue ;;
      esac
    fi
    _sane=0
    error_print "\$lib not found."
  done
  #---------------------------------------------------

  #+++++++++++++++++++++++++++++++++++++++++++++++++++
  # Ensure we're on a supported machine
  debug_print 'Checking machine architecture...'
  _machine="\$(uname -m)"
  debug_print "Machine type is \$_machine"
  if [ "\$_machine" != "\$ARCH" ]; then
    _sane=0
    error_print "\$_machine is not supported."
  fi
  #---------------------------------------------------

  rm -f "\$_errout"
  # Exit if one or more sanity checks fail
  [ "\$_sane" = 0 ] && error_print 'Pre-check failed!' && exit 1
  info_print 'All checks succeeded.'
}

MS_Decompress()
{
  eval "$GUNZIP_CMD"

  if test \$? -ne 0; then
    echo " ... Decompression failed." >&2
  fi
}

UnTAR()
{
  if test "\$quiet" = n; then
    tar \$1vf - $UNTAR_EXTRA 2>&1 || { echo " ... Extraction failed." >&2; kill -15 \$$; }
  else
    tar \$1f - $UNTAR_EXTRA 2>&1 || { echo Extraction failed. >&2; kill -15 \$$; }
  fi
}

MS_cleanup()
{
  echo 'Signal caught, cleaning up' >&2
  cd "\$TMPROOT"
  rm -rf "\$tmpdir"
  eval \$finish; exit 15
}

MS_showInfo() {
  MS_Version_show
  echo Identification: "\$label"
  echo Extract directory: "\$extractdir"
  echo Uncompressed size: $USIZE KB
  echo Compression: $COMPRESS
  echo Total archive size: \$(($totalsize / 1024)) KB
  if test -n "$ENCRYPT"; then echo Encryption: $ENCRYPT; fi
  echo Date of packaging: $DATE
  echo Built with Makeself version $MS_VERSION
  if test -n "\$script"; then
    echo Script run after extraction:
    echo "    " \$script \$scriptargs
  fi
  if test "$copy" = copy; then
    echo "Archive will copy itself to a temporary location"
  fi
  if test "$KEEP" = y; then
    echo "directory \$extractdir is permanent"
  else
    echo "\$extractdir will be removed after extraction"
  fi
  MS_Printf '\\nHash type: SHA-256\\n'
  echo Archive Checksum: \$SHA
  MS_Printf '\\nExtra info:\\n'
  echo totalsize=\"\$totalsize\"
  echo filesizes=\"\$filesizes\"
  echo NOOVERWRITE=$NOOVERWRITE
  echo SKIP=\"\$skip\"
  echo
}

finish=true
noprogress=$NOPROGRESS
copy=$COPY
ownership=$OWNERSHIP
verbose=n
cleanup=y
sig_key=
noask=n
noexec=n
enlist=n
check=n

initargs="\$@"

while [ \$# -gt 0 ]; do
  case "\$1" in
    install ) shift ;;
    uninstall | remove )  script="./uninstall"; shift ;;
    --assumeyes )  noask=y; scriptargs="\$scriptargs -y"; shift ;;
    --check )  check=y; shift ;;
    --chown ) ownership=y; shift ;;
    --exdir ) keep=y; extractdir="\${2:-.}"; shift 2 || { MS_Help; exit 1; };;
    --help ) MS_Help; exit 0 ;;
    --info | --version ) MS_showInfo; exit 0;;
    --keep ) keep=y; shift ;;
    --list )   enlist=y; shift ;;
    --nodiskspace ) nodiskspace=y; shift ;;
    --nochown ) ownership=n; shift ;;
    --noexec )  noexec=y; shift;;
    --noprogress )  noprogress=y; shift ;;
    --phase2 )  copy=phase2; shift ;;
    --quiet )  quiet=y; noprogress=y; scriptargs="\$scriptargs -q"; shift ;;
    --verbose )    verbose=y; scriptargs="\$scriptargs -v"; shift ;;
    --show-file )
      _file="\$2"
      shift 2 || { printf "File required.\\n"; MS_Help; exit 1; }
      offset=\$(head -n "\$skip" "\$0" | wc -c | sed "s/ //g")
      { MS_dd "\$0" \$offset \$totalsize | MS_Decompress | \\
        tar xOf - "./\${_file#./}"; } && exit 0
      MS_Printf \$_file not found in archive.
      exit 1
      ;;
    --tar )
      offset=\$(head -n "\$skip" "\$0" | wc -c | sed "s/ //g")
      arg1="\$2"
        shift 2 || { MS_Help; exit 1; }
      for s in \$filesizes; do
        MS_dd "\$0" \$offset \$s | MS_Decompress | tar "\$arg1" - "\$@"
        offset=\$((offset + s))
      done
      exit 0
      ;;
    --verify-sig)
      sig_key="\$2"
      shift 2 || { MS_Help; exit 1; }
      MS_Verify_Sig "\$0"
      ;;
    -*)
      OPTIND=1
      while getopts :chklnqVvx:y- opt; do
        case \$opt in
          c) check=y;;
          h) MS_Help; exit 0;;
          k) keep=y;;
          l) enlist=y;;
          n) noexec=y;;
          q) quiet=y; noprogress=y; scriptargs="\$scriptargs -q";;
          V) MS_showInfo; exit 0;;
          v) verbose=y; scriptargs="\$scriptargs -v";;
          x) keep=y; extractdir="\${OPTARG:-.}";;
          y) noask=y; scriptargs="\$scriptargs -y";;
          -) break;;
         \?) MS_Printf Unrecognized flag : "\$opt" >&2; MS_Help; exit 1;;
        esac
      done
      if [ "\$opt" = - ]; then shift; else shift \$((OPTIND - 1)); fi
      ;;
    *) MS_Printf Unrecognized flag : "\$1" >&2; MS_Help; exit 1;;
  esac
done

[ "\$debug" = 1 ] && verbose=y
[ "\$noexec" = y ] && script=""
if test "\$quiet" = y -a "\$verbose" = y; then quiet=n; fi
if test "\$quiet" = y -o "\$noexec" = y; then noask=y; fi

if test "\$check" = y; then MS_Check "\$0" y; exit 0; fi

if test "\$enlist" = y; then
  echo Extract directory: \$extractdir
  offset=\$(head -n "\$skip" "\$0" | wc -c | sed "s/ //g")
  for s in \$filesizes; do
    MS_dd "\$0" \$offset \$s | MS_Decompress | UnTAR t
    offset=\$((offset + s))
  done
  exit 0
fi

case "\$copy" in
  copy )
    tmpdir="\$(mktemp -d "\$TMPROOT/makeself.XXXXXX" || \\
      exit 1)"
    SCRIPT_COPY="\$tmpdir/makeself"
    echo "Copying to a temporary location..." >&2
    cp "\$0" "\$SCRIPT_COPY"
    chmod +x "\$SCRIPT_COPY"
    cd "\$TMPROOT"
    export USER_PWD="\$tmpdir"
    exec "\$SCRIPT_COPY" --phase2 \$initargs
    ;;
  phase2 )
    finish="\$finish ; rm -rf \${0%/*}"
    ;;
esac

if test "\$extractdir" = .; then
    tmpdir="."
else
  if test "\$keep" = y; then
    if test "\$nooverwrite" = y && test -d "\$extractdir"; then
      echo "Extract directory \$extractdir already exists, aborting." >&2
      exit 1
    fi
    if test "\$quiet" = n; then echo "Creating directory \$extractdir"; fi
    tmpdir="\$extractdir"
    dashp="-p"
  else
    dashp=""
    tmpdir="\$(mktemp -u "\$TMPROOT/selfgz.XXXXXX")"
  fi
  mkdir \$dashp "\$tmpdir" || {
    echo 'Cannot create extract directory' \$tmpdir >&2
    echo 'You should try option --exdir dir' >&2
    eval \$finish
    exit 1
  }
fi

if test "\$SETUP_NOCHECK" != 1; then MS_Check "\$0"; fi
offset=\$(head -n "\$skip" "\$0" | wc -c | sed "s/ //g")

if test "\$noexec" = n; then MS_Preextract; fi

if test "\$noask" = n; then
  MS_Printf "About to extract $USIZE KB in \$tmpdir ...\n"
  read -p "Proceed? [Y/n] " -r ans
  if test "\$ans" = n; then eval \$finish; exit 1; fi
fi

[ "\$quiet" = n ] && MS_Printf "Uncompressing \$label"
res=3
[ "\$keep" = n ] && trap MS_cleanup 1 2 3 15

if test "\$nodiskspace" = n; then
  leftspace=\$(MS_diskspace "\$tmpdir")
  if test -n "\$leftspace"; then
    if test "\$leftspace" -lt $USIZE; then
      echo
      echo "Not enough space left in "\${tmpdir%/*}" (\$leftspace KB) to decompress \$0 ($USIZE KB)" >&2
      echo "Use --nodiskspace option to skip this check and proceed anyway" >&2
      if test "\$keep" = n; then
        echo "Consider setting TMPDIR to a directory with more free space."
      fi
      eval \$finish
      exit 1
    fi
  fi
fi

for s in \$filesizes
do
  if MS_dd_Progress "\$0" \$offset \$s | MS_Decompress | \\
    ( cd "\$tmpdir"; umask \$ORIG_UMASK ; UnTAR xp ) >/dev/null; then
    if test "\$ownership" = y; then
      (cd "\$tmpdir"; chown -R \$(id -u) .; chgrp -R \$(id -g) .)
    fi
  else
    echo >&2
    echo "Unable to decompress \$0" >&2
    eval \$finish; exit 1
  fi
  offset=\$((offset + s))
done
if test "\$quiet" = n; then echo; fi

cd "\$tmpdir"
res=0
if test -n "\$script"; then
  if test "\$export_conf" = y; then
    MS_BUNDLE="\$0"
    MS_LABEL="\$label"
    MS_SCRIPT="\$script"
    MS_SCRIPTARGS="\$scriptargs"
    MS_ARCHDIRNAME="\$archdirname"
    MS_KEEP="\$KEEP"
    MS_NOOVERWRITE="\$NOOVERWRITE"
    MS_COMPRESS="\$COMPRESS"
    MS_CLEANUP="\$cleanup"
    export MS_BUNDLE MS_LABEL MS_SCRIPT MS_SCRIPTARGS
    export MS_ARCHDIRNAME MS_KEEP MS_NOOVERWRITE MS_COMPRESS
  fi

  if test "\$verbose" = y; then
    MS_Printf "Executing: \$script \$scriptargs \$* ...\n"
  fi
  eval "\"\$script\" \$scriptargs \"\\\$@\""; res=\$?
  if test "\$res" -ne 0; then
    test "\$verbose" = y && \
    echo "The program '\$script' returned an error code (\$res)" >&2
  fi
fi

if test "\$keep" = n; then cd "\$TMPROOT"; rm -rf "\$tmpdir"; fi
eval \$finish
exit \$res
EOF
