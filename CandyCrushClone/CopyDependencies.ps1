param (
  [string]$ProjectDir,
  [string]$TargetDir,
  [string]$PlatformShortName
)

robocopy "$($ProjectDir)Assets" "$($TargetDir)Assets" /mir /log:$($TargetDir)robocopy.log
robocopy "$($ProjectDir)../SDL/lib/$($PlatformShortName)" "$TargetDir" SDL2.dll /log+:$($TargetDir)robocopy.log
robocopy "$($ProjectDir)../SDLImage/lib/$($PlatformShortName)" "$TargetDir" SDL2_image.dll /log+:$($TargetDir)robocopy.log
robocopy "$($ProjectDir)../SDLTTF/lib/$($PlatformShortName)" "$TargetDir" SDL2_ttf.dll /log+:$($TargetDir)robocopy.log
robocopy "$($ProjectDir)../SDLMixer/lib/$($PlatformShortName)" "$TargetDir" SDL2_mixer.dll /log+:$($TargetDir)robocopy.log