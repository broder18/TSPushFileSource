#pragma once

#define RETURN_FAILED_HR    if(FAILED(Hr)) return Hr
#define EXEC_CHECK_HR(x)    { HRESULT Hr = x; RETURN_FAILED_HR; }
#define _EXEC_CHECK_HR(x)   Hr = x; RETURN_FAILED_HR;

#define BUFANDSIZE(x)       x, sizeof(x)
#define QDELETE(x)          if(x) { delete x; x = 0; }
