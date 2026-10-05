# RLink integration

RLink vendors the official libjpeg-turbo 3.2.0 source release and builds the
TurboJPEG API with the same MSVC runtime as the application.

- Upstream: https://github.com/libjpeg-turbo/libjpeg-turbo
- Release asset: `libjpeg-turbo-3.2.0.tar.gz`
- SHA-256: `6f30092cef9fb839779646608f4ee14ae3cbac989c47fa05e841b0841f09878e`
- RLink target: upstream `turbojpeg` DLL, built with `/MD`

The source archive is the supported upstream release artifact. Its SHA-256 was
matched before extraction.
