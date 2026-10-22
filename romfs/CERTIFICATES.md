# HTTPS certificate store

`cacert.pem` is the unmodified Mozilla root certificate bundle distributed by
https://curl.se/ca/cacert.pem (Mozilla data dated 20 October 2026).
SHA256: `796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe`.

Source/extraction: https://curl.se/docs/caextract.html
Mozilla source: https://github.com/mozilla-firefox/firefox/tree/release/security/nss/lib/ckfw/builtins
Mozilla Public License 2.0: https://www.mozilla.org/MPL/2.0/

PaperNX embeds this file in RomFS for libcurl's verified HTTPS connections on
Switch. It contains public CA certificates, with no session proxy certificates.
