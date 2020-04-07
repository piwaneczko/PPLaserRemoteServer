$Subject = "E=p.iwaneczko@gmai.com CN=Pawel Iwaneczko S=Poland C=PL"

#$cacert = (Get-ChildItem Cert:\CurrentUser\Root\thumbprint)

#$cert = New-SelfSignedCertificate -Subject $Subject -CertStoreLocation Cert:\CurrentUser\My\ -Type Codesigning
$cert = (Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert)

Export-Certificate -Cert $cert -FilePath ppremote.cer

#Set-AuthenticodeSignature .\ppremote.cer -Certificate $cacert
Set-AuthenticodeSignature .\ppremote.cer -Certificate $cert