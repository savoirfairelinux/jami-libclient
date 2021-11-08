#!/bin/bash

echo ""
cd build-local
/Applications/Xcode.app/Contents/SharedFrameworks/ContentDeliveryServices.framework/Versions/A/Frameworks/AppStoreService.framework/Versions/A/Support/altool --notarize-app -t osx -f Jami.app.zip --primary-bundle-id ${BUNDLE_ID} -u ${APPLE_ACCOUNT} -p ${APPLE_PASSWORD} --output-format xml -itc_provider ${TEAM_ID} > UploadInfo.plist
REQUESTID=$(xmllint --xpath "/plist/dict[key='notarization-upload']/dict/key[.='RequestUUID']/following-sibling::string[1]/node()" UploadInfo.plist)
echo "file uploaded for notarization"
echo ${REQUESTID}
sleep 60
x=1
while [ $x -le 15 ];
do
/Applications/Xcode.app/Contents/SharedFrameworks/ContentDeliveryServices.framework/Versions/A/Frameworks/AppStoreService.framework/Versions/A/Support/altool --notarization-info ${REQUESTID} -u ${APPLE_ACCOUNT}  -p ${APPLE_PASSWORD}  --output-format xml > RequestedInfo.plist
ANSWER=$(xmllint --xpath "/plist/dict[key='notarization-info']/dict/key[.='Status']/following-sibling::string[1]/node()" RequestedInfo.plist)
if [ "$ANSWER" == "in progress" ];
then
echo  "notarization in progress"
sleep 60
x=$(( $x + 1 ))
elif [ "$ANSWER" == "success" ]
then
echo  "notarization success"
break
else
echo "notarization failed"
break
exit 1
fi
done
ANSWER=$(xmllint --xpath "/plist/dict[key='notarization-info']/dict/key[.='Status']/following-sibling::string[1]/node()" RequestedInfo.plist)
if [ "$ANSWER" != "success" ];
then
echo "notarization failed"
exit 1
fi
