$Subject = "CN=Pawel Iwaneczko, 
            E=p.iwaneczko@gmail.com,
            OU=https://github.com/piwaneczko/PPLaserRemoteServer/releases,
            S=Poland,
            C=PL"
#$cacert = (Get-ChildItem Cert:\CurrentUser\Root\thumbprint)

#New-SelfSignedCertificate -Subject $Subject -CertStoreLocation Cert:\CurrentUser\My\ -Type Codesigning
$cert = (Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert)

Export-Certificate -Cert $cert -FilePath ppremote.cer

#Set-AuthenticodeSignature .\ppremote.cer -Certificate $cacert
Set-AuthenticodeSignature .\ppremote.cer -Certificate $cert