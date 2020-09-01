# 사용자 계정 구하는 방법 (how to get user account)

미니필터 드라이버에서 기존에는 SID를 구하는 방법을 가지고 사용자를 판별하였다면,  
이번에는 그 SID를 가지고 사용자 계정을 구하는 방법에 대해서 설명하도록 하겠습니다.  
먼저 사용자 계정을 구하기 위해서 필요한 구조체와 API는 다음과 같습니다.  

``` C
typedef struct _SID {
  UCHAR                    Revision;
  UCHAR                    SubAuthorityCount;
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
#if ...
  ULONG                    *SubAuthority[];
#else
  ULONG                    SubAuthority[ANYSIZE_ARRAY];
#endif
} SID, *PISID;

typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown,
    SidTypeComputer,
    SidTypeLabel,
    SidTypeLogonSession
} SID_NAME_USE, *PSID_NAME_USE;

KSECDDDECLSPEC NTSTATUS SEC_ENTRY SecLookupAccountSid(
  PSID            Sid,
  PULONG          NameSize,
  PUNICODE_STRING NameBuffer,
  PULONG          DomainSize,
  PUNICODE_STRING DomainBuffer,
  PSID_NAME_USE   NameUse
);
```

사용자 계정을 구하기 위해서는 기존에 구했던 SID를 첫번째 Sid로 넘겨주면서   
SecLookupAccountSid API를 호출하면 됩니다.  

```C
if (NT_SUCCESS(Status)) {
  SId = (SID*)pUser->User.Sid;
  Status = SecLookupAccountSid(SId, &NameSize, NULL, NULL, NULL, &NameUse);
  if (Status == STATUS_BUFFER_TOO_SMALL) {
    UniName.Length = (USHORT)(NameSize * sizeof(WCHAR));
    UniName.MaximumLength = (USHORT)(NameSize * sizeof(WCHAR));
    UniName.Buffer = MyAllocNonPagedPool(NameSize * sizeof(WCHAR), &g_NonPagedPoolCnt);

    Status = SecLookupAccountSid(SId, &NameSize, &UniName, NULL, NULL, &NameUse);
  if (NT_SUCCESS(Status)) {
  	DbgPrint("[TEST] Name[%S]", UniName.Buffer);
  }
  else DbgPrint("[TEST] SecLookupAccountSid failed [0x%X]", Status);

  if (UniName.Buffer) MyFreeNonPagedPool(UniName.Buffer, &g_NonPagedPoolCnt);
}
```

위 소스코드에서 보면 처음 해당 SID의 계정명 버퍼 사이즈를 모르기 때문에   
3번째 NameBuffer에 넘겨줄 때 NULL 로 셋팅해서 넘겨주면 NameSize에 버퍼 사이즈가   
셋팅되어 넘어오면서 에러코드는 STATUS_BUFFER_TOO_SMALL 가 넘어오게 됩니다.  
그러면 그걸 구분한 뒤 넘겨 받은 버퍼 사이즈만큼 버퍼를 할당하여 다시 SecLookupAccountSid API를   
호출하면 해당 SID의 계정명을 손쉽게 획득할 수 있습니다.  
그리고 마지막에는 사용 후 반드시 할당한 NameBuffer를 해제해 주면 됩니다.  