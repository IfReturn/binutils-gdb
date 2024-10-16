#name: GNU Property (input without gcs output forced with gcs gcs-report=warning)
#source: property-gcs.s
#alltargets: [check_shared_lib_support] *linux*
#as: -march=armv9.4-a+gcs -defsym __mult__=0
#ld: -z gcs -z gcs-report=warning
#readelf: -n
#warning: .*property-gcs.*: warning: GCS turned on by -z gcs on the output when all inputs do not have GCS in NOTE section.

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: AArch64 feature: GCS
