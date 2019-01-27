$Subject = "E=p.iwaneczko@aircom.ag
            OU=www.aircom.ag
            CN=Aircom AI"

$cacert = (Get-ChildItem Cert:\CurrentUser\Root\135CEC36F49CB8E93B1AB270CD80884676CE8F33)

$Cert = New-SelfSignedCertificate -Subject $Subject -CertStoreLocation Cert:\CurrentUser\My\ -Type Codesigning

Export-Certificate -Cert $Cert -FilePath ppremote.cer

Set-AuthenticodeSignature .\ppremote.cer -Certificate $cacert