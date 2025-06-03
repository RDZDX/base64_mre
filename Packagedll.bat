"C:\Program Files\MRE_SDK\tools\DllPackage.exe" "D:\MyGitHub\base64_mre\base64_mre.vcproj"
if %errorlevel% == 0 (
 echo postbuild OK.
  copy base64_mre.vpp ..\..\..\MoDIS_VC9\WIN32FS\DRIVE_E\base64_mre.vpp /y
exit 0
)else (
echo postbuild error
  echo error code: %errorlevel%
  exit 1
)

