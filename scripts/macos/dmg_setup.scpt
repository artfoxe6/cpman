-- macOS DMG setup script for CPack
-- This AppleScript sets up the DMG layout

on run argv
    set volumeName to item 1 of argv
    
    tell application "Finder"
        set theWindow to container window of disk volumeName
        set current view of theWindow to icon view
        set toolbar visible of theWindow to false
        set statusbar visible of theWindow to false
        set the bounds of theWindow to {400, 100, 900, 500}
        set theViewOptions to the icon view options of theWindow
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 96
        
        -- Position the app in the center
        set position of item "cpman.app" of theWindow to {250, 200}
        
        -- Create Applications symlink
        make new alias at theWindow to POSIX file "/Applications" with properties {name:"Applications"}
        set position of item "Applications" of theWindow to {500, 200}
        
        close theWindow
        open theWindow
        update without registering applications
    end tell
end run