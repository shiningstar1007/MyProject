#사용자 SID 구하는 방법 (How to get user SID?)

커널에서 현재 사용자를 구하는 방법은 2가지가 있습니다.  
첫번째는 사용자의 SID를 구하는 방법, 두번째는 사용자의 계정명을 구하는 방법입니다.  
미니필터 드라이버에서는 둘 다 획득하는 방법이 있는데, 먼저 SID를 구하는 방법을 소개해 보려고 합니다.  
먼저 호출해야 할 API들과 파라미터들을 보겠습니다.  

``` C
typedef PVOID PACCESS_TOKEN;

typedef struct _TOKEN_USER { 
    SID_AND_ATTRIBUTES User; 
} TOKEN_USER, *PTOKEN_USER;

void SeQuerySubjectContextToken(
	SubjectContext
);

NTSTATUS SeQueryInformationToken(
	PACCESS_TOKEN Token,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	PVOID *TokenInformation
);

VOID GetUserSID(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PACCESS_TOKEN pToken = NULL;
	PTOKEN_USER pUser = NULL;

	pToken = SeQuerySubjectContextToken(&(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext));
	if (pToken) {
		Status = SeQueryInformationToken(pToken, TokenUser, &pUser);
	}
    
	if (NT_SUCCESS(Status)) {
		pUser->User.Sid;
		ExFreePool(pUser);
	}
}
```

먼저 현재 AccessToken을 구하기 위해서 SeQuerySubjectContextToken API를 호출해야 합니다.  
이 API를 호출 할 때 필요한 SubjectContext를 넘겨주기 위해서는 미니필터에서 콜백으로 들어온  
PFLT_CALLBACK_DATA 내 SubjectContext를 넘겨주면 됩니다.  
이 후 AccessToken 을 획득 하면 이 후 SeQueryInformationToken API를 호출하는데  
TOKEN_USER 구조체 변수를 포인터로 선언해서 파라미터로 함께 넘겨주면 됩니다.  
그래서 성공하면 SeQueryInformationToken API 내에서 버퍼를 할당하여  
정보를 넘겨주게 됨으로써 User SID를 획득 할 수 있습니다.  
그리고 SeQueryInformationToken API를 호출한 쪽에서는   
반드시 넘겨 받은 TokenInformation을 해제 해줘야 합니다.  