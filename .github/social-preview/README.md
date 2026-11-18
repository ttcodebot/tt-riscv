# Social preview

`social-preview.png` (1280x640) is the image GitHub shows when the repository is linked. Upload it in the
repository settings (Settings -> General -> Social preview); GitHub has no API for it.

`social.html` is the source: render it at 1280x640 (e.g. a Playwright screenshot of the page) with the Lato
and DejaVu Sans Mono fonts installed. The logo is `tinytapeout_www/static/ttlogo.svg`; the background
assembly is RISC-V code that drives the 7-segment display through the KianV GPIO registers.
