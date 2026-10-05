param(
    [string]$FfmpegSource = 'E:\webrtc_src\src\third_party\ffmpeg',
    [string]$MsysRoot = 'D:\msys64',
    [string]$X264Prefix = '',
    [string]$OneVplPrefix = '',
    [string]$NvCodecHeadersPrefix = '',
    [string]$AmfIncludeRoot = '',
    [switch]$RequireHardwareEncoders,
    [int]$Jobs = 4
)

$ErrorActionPreference = 'Stop'

function Convert-ToMsysPath([string]$Path) {
    $resolved = [System.IO.Path]::GetFullPath($Path)
    if ($resolved -notmatch '^([A-Za-z]):\\(.*)$') {
        throw "Unsupported Windows path: $resolved"
    }
    $drive = $Matches[1].ToLowerInvariant()
    $tail = $Matches[2].Replace('\', '/')
    return "/$drive/$tail"
}

$repositoryRoot = [System.IO.Path]::GetFullPath(
    (Join-Path $PSScriptRoot '..'))
$dependencyRoot = Join-Path $repositoryRoot 'third_party\ffmpeg_d3d11va'
$buildRoot = Join-Path $dependencyRoot 'build'
$prefixRoot = Join-Path $dependencyRoot 'prefix'
$temporaryRoot = Join-Path $dependencyRoot 'tmp'
$homeRoot = Join-Path $dependencyRoot 'home'
$bash = Join-Path $MsysRoot 'usr\bin\bash.exe'
$winPthread = Join-Path $MsysRoot 'mingw64\bin\libwinpthread-1.dll'
$X264Prefix = if ([string]::IsNullOrWhiteSpace($X264Prefix)) {
    $candidates = @(
        (Join-Path $repositoryRoot 'third_party\x264\prefix'),
        (Join-Path $MsysRoot 'home\dyh17\ffmpeg\build\libx264')
    )
    $candidates | Where-Object {
        Test-Path -LiteralPath (Join-Path $_ 'lib\pkgconfig\x264.pc')
    } | Select-Object -First 1
} else {
    [System.IO.Path]::GetFullPath($X264Prefix)
}
$OneVplPrefix = if ([string]::IsNullOrWhiteSpace($OneVplPrefix)) {
    @(
        (Join-Path $repositoryRoot 'third_party\onevpl\prefix'),
        (Join-Path $MsysRoot 'mingw64')
    ) | Where-Object {
        Test-Path -LiteralPath (Join-Path $_ 'lib\pkgconfig\vpl.pc')
    } | Select-Object -First 1
} else {
    [System.IO.Path]::GetFullPath($OneVplPrefix)
}
$NvCodecHeadersPrefix = if ([string]::IsNullOrWhiteSpace(
        $NvCodecHeadersPrefix)) {
    @(
        (Join-Path $repositoryRoot 'third_party\nv-codec-headers\prefix'),
        (Join-Path $MsysRoot 'mingw64')
    ) | Where-Object {
        Test-Path -LiteralPath (Join-Path $_ 'lib\pkgconfig\ffnvcodec.pc')
    } | Select-Object -First 1
} else {
    [System.IO.Path]::GetFullPath($NvCodecHeadersPrefix)
}
$AmfIncludeRoot = if ([string]::IsNullOrWhiteSpace($AmfIncludeRoot)) {
    @(
        (Join-Path $repositoryRoot 'third_party\amf\include'),
        (Join-Path $MsysRoot 'mingw64\include')
    ) | Where-Object {
        Test-Path -LiteralPath (Join-Path $_ 'AMF\core\Version.h')
    } | Select-Object -First 1
} else {
    [System.IO.Path]::GetFullPath($AmfIncludeRoot)
}
$hardwareInputs = @{
    QSV = -not [string]::IsNullOrWhiteSpace($OneVplPrefix)
    NVENC = -not [string]::IsNullOrWhiteSpace($NvCodecHeadersPrefix)
    AMF = -not [string]::IsNullOrWhiteSpace($AmfIncludeRoot)
}
if ($RequireHardwareEncoders -and
    ($hardwareInputs.Values | Where-Object { -not $_ }).Count -ne 0) {
    $missing = @($hardwareInputs.GetEnumerator() |
        Where-Object { -not $_.Value } |
        ForEach-Object { $_.Key }) -join ', '
    throw "Required FFmpeg hardware encoder build inputs are missing: $missing"
}

foreach ($required in @(
    $bash,
    $winPthread,
    (Join-Path $FfmpegSource 'configure'),
    (Join-Path $X264Prefix 'lib\pkgconfig\x264.pc')
)) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Required FFmpeg build input is missing: $required"
    }
}
foreach ($directory in @($buildRoot, $prefixRoot, $temporaryRoot, $homeRoot)) {
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
}

# Patch a workspace-local copy; never modify the WebRTC dependency checkout.
# Robocopy preserves source mtimes so unchanged files remain incremental.
$preparedSource = Join-Path $dependencyRoot 'source'
New-Item -ItemType Directory -Force -Path $preparedSource | Out-Null
& robocopy.exe $FfmpegSource $preparedSource /E /XJ /XD .git /NFL /NDL /NJH /NJS /NP
if ($LASTEXITCODE -ge 8) { throw 'Could not prepare the FFmpeg source copy.' }
# The local copy lives inside RLink's Git checkout. Pin VERSION explicitly so
# FFmpeg's version.sh cannot accidentally report RLink's parent commit.
if (Test-Path -LiteralPath (Join-Path $FfmpegSource 'VERSION')) {
    $sourceVersion = (Get-Content -LiteralPath (Join-Path $FfmpegSource 'VERSION') -Raw).Trim()
} elseif (Test-Path -LiteralPath (Join-Path $FfmpegSource '.git')) {
    $sourceVersion = (& git -c "safe.directory=$FfmpegSource" -C $FfmpegSource log -1 --format=git-%cd-%h --date=short).Trim()
    if ($LASTEXITCODE -ne 0) { throw 'Could not identify the FFmpeg source revision.' }
} else {
    $sourceVersion = (Get-Content -LiteralPath (Join-Path $FfmpegSource 'RELEASE') -Raw).Trim()
}
[System.IO.File]::WriteAllText((Join-Path $preparedSource 'VERSION'),
    "$sourceVersion-rlink-nvenc-fps1`n", [System.Text.UTF8Encoding]::new($false))
Push-Location $repositoryRoot
try {
    $patchPath = Join-Path $repositoryRoot 'patches\ffmpeg\nvenc-dynamic-fps.patch'
    & git apply --check --ignore-space-change --directory=third_party/ffmpeg_d3d11va/source $patchPath
    if ($LASTEXITCODE -ne 0) { throw 'NVENC patch does not match this FFmpeg revision.' }
    & git apply --ignore-space-change --directory=third_party/ffmpeg_d3d11va/source $patchPath
    if ($LASTEXITCODE -ne 0) { throw 'Could not apply the NVENC dynamic FPS patch.' }
} finally { Pop-Location }
$FfmpegSource = $preparedSource

$sourceMsys = Convert-ToMsysPath $FfmpegSource
$buildMsys = Convert-ToMsysPath $buildRoot
$prefixMsys = Convert-ToMsysPath $prefixRoot
$temporaryMsys = Convert-ToMsysPath $temporaryRoot
$homeMsys = Convert-ToMsysPath $homeRoot
$x264PkgConfigMsys = Convert-ToMsysPath(
    (Join-Path $X264Prefix 'lib\pkgconfig'))
$pkgConfigPaths = @($x264PkgConfigMsys)
if ($hardwareInputs.QSV) {
    $pkgConfigPaths += Convert-ToMsysPath(
        (Join-Path $OneVplPrefix 'lib\pkgconfig'))
}
if ($hardwareInputs.NVENC) {
    $nvEncodeHeader = Join-Path $NvCodecHeadersPrefix `
        'include\ffnvcodec\nvEncodeAPI.h'
    $nvEncodeHeaderText = Get-Content -LiteralPath $nvEncodeHeader -Raw
    if ($nvEncodeHeaderText -notmatch `
            '#define\s+NVENCAPI_MAJOR_VERSION\s+12' -or
        $nvEncodeHeaderText -notmatch `
            '#define\s+NVENCAPI_MINOR_VERSION\s+2' -or
        $nvEncodeHeaderText -notmatch `
            'uint32_t\s+countingType\s*:\s*1') {
        throw (
            'The bundled FFmpeg 62 source requires FFmpeg ' +
            'nv-codec-headers n12.2.72.0. Install that official tag ' +
            "under $NvCodecHeadersPrefix before rebuilding.")
    }
    $pkgConfigPaths += Convert-ToMsysPath(
        (Join-Path $NvCodecHeadersPrefix 'lib\pkgconfig'))
}
$extraCFlags = '-O2'
if ($hardwareInputs.AMF) {
    $extraCFlags += ' -I' + (Convert-ToMsysPath $AmfIncludeRoot)
}
$environment =
    "export PATH=/mingw64/bin:/usr/bin; " +
    "export TMP='$temporaryMsys'; export TEMP='$temporaryMsys'; " +
    "export TMPDIR='$temporaryMsys'; export HOME='$homeMsys'; " +
    "export PKG_CONFIG_PATH='$($pkgConfigPaths -join ':')'; " +
    "cd '$buildMsys'; "

$configure =
    $environment +
    "'$sourceMsys/configure' " +
    "--prefix='$prefixMsys' --target-os=mingw32 --arch=x86_64 " +
    "--enable-shared --disable-static --disable-programs --disable-doc " +
    "--disable-debug --disable-network --disable-autodetect " +
    "--disable-everything --disable-avdevice --disable-avfilter " +
    "--disable-avformat --disable-swresample --disable-swscale " +
    "--enable-gpl --enable-libx264 " +
    "--enable-avcodec --enable-avutil --enable-decoder=h264 " +
    "--enable-encoder=libx264 " +
    "--enable-parser=h264 --enable-d3d11va " +
    "--enable-hwaccel=h264_d3d11va " +
    "--enable-hwaccel=h264_d3d11va2 "
if ($hardwareInputs.QSV) {
    $configure += "--enable-libvpl --enable-encoder=h264_qsv "
}
if ($hardwareInputs.NVENC) {
    $configure += "--enable-ffnvcodec --enable-nvenc " +
        "--enable-encoder=h264_nvenc "
}
if ($hardwareInputs.AMF) {
    $configure += "--enable-amf --enable-encoder=h264_amf "
}
$configure += "--extra-cflags='$extraCFlags'"

& $bash -lc $configure
if ($LASTEXITCODE -ne 0) { throw 'FFmpeg configure failed.' }
& $bash -lc ($environment + "make -j$Jobs")
if ($LASTEXITCODE -ne 0) { throw 'FFmpeg build failed.' }
& $bash -lc ($environment + 'make install')
if ($LASTEXITCODE -ne 0) { throw 'FFmpeg install failed.' }

Copy-Item -LiteralPath $winPthread `
    -Destination (Join-Path $prefixRoot 'bin\libwinpthread-1.dll') -Force
$x264Runtime = Get-ChildItem -LiteralPath (Join-Path $X264Prefix 'bin') `
    -Filter 'libx264-*.dll' -File | Select-Object -First 1
if (-not $x264Runtime) {
    throw "Expected x264 runtime was not found under $X264Prefix\bin."
}
Copy-Item -LiteralPath $x264Runtime.FullName `
    -Destination (Join-Path (Join-Path $prefixRoot 'bin') $x264Runtime.Name) -Force

$requiredOutputs = @(
    (Join-Path $prefixRoot 'bin\avcodec-62.dll'),
    (Join-Path $prefixRoot 'bin\avutil-60.dll'),
    (Join-Path $prefixRoot 'bin\libwinpthread-1.dll'),
    (Join-Path (Join-Path $prefixRoot 'bin') $x264Runtime.Name)
)
if ($hardwareInputs.QSV) {
    $vplRuntime = Get-ChildItem -LiteralPath (Join-Path $OneVplPrefix 'bin') `
        -Filter '*vpl*.dll' -File | Select-Object -First 1
    if (-not $vplRuntime) {
        throw "oneVPL pkg-config was found but its dispatcher DLL is missing."
    }
    Copy-Item -LiteralPath $vplRuntime.FullName `
        -Destination (Join-Path (Join-Path $prefixRoot 'bin') $vplRuntime.Name) -Force
    $requiredOutputs += Join-Path (Join-Path $prefixRoot 'bin') $vplRuntime.Name
}
foreach ($output in $requiredOutputs) {
    if (-not (Test-Path -LiteralPath $output)) {
        throw "Expected FFmpeg runtime was not produced: $output"
    }
}
Write-Host 'RemoteC FFmpeg H264 runtime is ready:'
$requiredOutputs | ForEach-Object { Write-Host "  $_" }
Write-Host ('FFmpeg hardware encoders: QSV={0}; NVENC={1}; AMF={2}' -f `
    $hardwareInputs.QSV, $hardwareInputs.NVENC, $hardwareInputs.AMF)
