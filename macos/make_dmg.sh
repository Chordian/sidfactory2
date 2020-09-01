#!/bin/sh
set -e

# remove existing dmg
rm -f "${DMG}"

# mount the temporary image
DEVICE=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_TMP}" | egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 5 

# layout the window (this doesn't work reliably yet)
# Make sure that "Enable access for assistive devices" is checked in System Preferences>>Universal Access. It is required for
# the AppleScript to work. You may have to reboot after this change.
#echo '
   #tell application "Finder"
     #tell disk "'${DMG_TITLE}'"
        #open
        #set current view of container window to icon view
        #delay 1
        #set toolbar visible of container window to false
        #delay 1
        #set statusbar visible of container window to false
        #delay 1
        #set theViewOptions to the icon view options of container window
        #delay 1
        #set arrangement of theViewOptions to not arranged
        #delay 1
        #set icon size of theViewOptions to 72
        #set the bounds of container window to {100, 100, 800, 800}
        #delay 2
        #set position of item "Install.txt" of container window to {55, 180}
        #delay 1
        #set position of item "'${APP_NAME}'" of container window to {255, 180}
        #delay 1
        #set position of item "'${APP_NAME}.app'" of container window to {255, 32}
        #delay 1
        #set position of item "Applications" of container window to {55, 32}
        #delay 1
        #set the bounds of container window to {100, 100, 450, 450}
        #delay 1
        #update without registering applications
        #close
        #open
        #delay 5
        #end tell
   #end tell
#' | osascript

# make read only
chmod -Rf go-w /Volumes/"${DMG_TITLE}"

# flush file cache
sync
sync

# unmount
hdiutil detach "${DEVICE}"

# make the final compressed DMG
hdiutil convert "${DMG_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG}"
