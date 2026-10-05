# NVENC dynamic FPS

`nvenc-dynamic-fps.patch` extends the bundled FFmpeg H.264 NVENC wrapper.
Baseline: WebRTC's FFmpeg checkout `ad41607c61898cf7150e0fb20fe4bbabd44922a3`.

Changing `AVCodecContext.framerate` updates frameRateNum/Den through
NvEncReconfigureEncoder on the existing session. The patch preserves the GOP,
profile, surface pool and timestamp base. FPS-only changes do not request a
rate-control reset or force an IDR; a driver may still produce an IDR. Combined
bitrate changes retain FFmpeg's existing reset behavior.

Configuration is prepared in a local snapshot and committed after SDK success.
Errors propagate to frame submission, with the acquired surface returned and
the device context popped. RLink then selects its existing software fallback.

The read-only `rlink_dynamic_fps` AVOption is initialized to 1 after successful
encoder initialization. RLink checks this marker and confirms the nominal FPS
only after successful submission/output processing. Older DLLs retain the
controlled-reopen path. The marker identifies wrapper support, not hardware
parameter readback or a guarantee that every driver accepts every update.

Build with `scripts/Build-FfmpegD3D11Va.ps1`. It copies sources into the ignored
`third_party/ffmpeg_d3d11va/source` directory, applies the patch, and builds the
existing DLL set; the external WebRTC checkout is not modified. VERSION records
the original FFmpeg revision plus `rlink-nvenc-fps1`.

Copy the updated RLinkAPP.exe, avcodec-62.dll and avutil-60.dll together when
testing on another machine. Other runtime dependencies are unchanged. No build
or driver upgrade is required on the target machine beyond the existing NVENC
runtime requirements.

Regression probe modes: `fps-only`, `fps-change`, `idle-recovery`. Use a matching
NVIDIA GPU for real SDK and bitrate validation. QSV uses its existing reset;
AMF remains on controlled reopen, and is not covered by this NVENC patch.
