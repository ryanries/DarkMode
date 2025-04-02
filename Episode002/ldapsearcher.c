// Dark Mode Episode 002
// https://www.youtube.com/playlist?list=PLlaINRtydtNUwkwdmCBNtkwgVRda8Ya_G
// Joseph Ryan Ries
#include <Windows.h>
#include <winldap.h>
#include <cpuid.h>
#include <stdio.h>
#pragma comment(lib, "wldap32.lib")

typedef unsigned int u32;
typedef unsigned long long u64;
typedef double f64;


u64 EstimateTSCFrequency(void)
{
	u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
	LARGE_INTEGER os_frequency = { 0 };
	QueryPerformanceFrequency(&os_frequency);
	// this will tell us the highest leaf this CPU supports.
	__get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
	if (eax >= 0x80000007)
	{
		// this will tell us whether the CPU has an invariant TSC.
		// it will be indicated by the ninth bit in EDX.
		__get_cpuid(0x80000007, &eax, &ebx, &ecx, &edx);

		if (edx & (1 << 8))
		{
			printf("CPUID reports an invariant TSC.\n");
		}
		else
		{
			printf("WARNING: This CPU does not have an invariant TSC! Using the OS default, which is NOT good.\n");
			return((u64)os_frequency.QuadPart);
		}
	}
	else
	{
		printf("WARNING: This CPU cannot tell us whether it has an invariant TSC! Using the OS default, which is NOT good.\n");
		return((u64)os_frequency.QuadPart);
	}

	//  time_in_seconds = number_of_clock_cycles / frequency	
	u64 start_tsc = 0, end_tsc = 0, elapsed_tsc = 0;
	u64 start_tick = 0, end_tick = 0;

	start_tsc = __rdtsc();
	QueryPerformanceCounter((LARGE_INTEGER*)&start_tick);
	do
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&end_tick);
	} while ((end_tick - start_tick) < (u64)os_frequency.QuadPart);
	end_tsc = __rdtsc();
	elapsed_tsc = end_tsc - start_tsc;
	printf("OS Frequency: %lluMHz\nEstimated TSC frequency: %lluMHz (%.2fGHz)\n",	         	
	         os_frequency.QuadPart / 1000000, elapsed_tsc / 1000000, (f64)elapsed_tsc / 1e9);
	return(elapsed_tsc);
}


int main(int argc, char* argv[])
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  u64 tsc_frequency = 0;
  u64 frame_start = 0;
  u64 frame_end = 0;
  u64 frame_elapsed = 0;
  u64 frame_elapsed_accumulated = 0;
  const u32 iterations = 500;

  tsc_frequency = EstimateTSCFrequency();
  
  LDAP* ld = ldap_initW(L"localhost", 389);
  ULONG ver = LDAP_VERSION3;
  ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ver);
  ULONG res = ldap_connect(ld, NULL);
  if (res != LDAP_SUCCESS)
  {
    printf("ldap_connect failed! (0x%08lx)\n", res);
    return(1);
  }
  res = ldap_bind_s(ld, "LAB\\Administrator", "Lab123!", LDAP_AUTH_SIMPLE);
  if (res != LDAP_SUCCESS)
  {
    printf("ldap_bind_s failed! (0x%08lx)\n", res);
    return(1);
  }


  // frame_start = __rdtsc();
  // Sleep(1000);
  // frame_end = __rdtsc();
  // frame_elapsed = frame_end - frame_start;
  // frame_elapsed *= 1000000;
  // frame_elapsed /= tsc_frequency;
  // printf("Sleep(1000) took %lld microseconds.\n", frame_elapsed);
  

  
  wchar_t* attrs = { L"sAMAccountName" };
  for (u32 i = 0; i < iterations; i++)
  {
    frame_start = __rdtsc();
    LDAPMessage* searchRes = NULL;
    res = ldap_search_sW(
      ld,
      L"CN=Terrence McSwiggan,CN=Users,DC=lab,DC=contoso,DC=com",
      LDAP_SCOPE_BASE,
      L"(objectClass=*)",
      &attrs,
      FALSE,
      &searchRes);
    if (res != LDAP_SUCCESS)
    {
      printf("ldap_search_sW failed! (0x%08lx)\n", res);
      ldap_unbind(ld);
      return(1);
    }
    if (searchRes)
    {
      ldap_msgfree(searchRes);
    }
    frame_end = __rdtsc();
    frame_elapsed = frame_end - frame_start;
    frame_elapsed *= 1000000;
    frame_elapsed /= tsc_frequency;
    frame_elapsed_accumulated += frame_elapsed;
  }  
  ldap_unbind(ld);
  printf("%d LDAP searches complete.\nAverage time take per search: %lld microseconds\n", iterations, (frame_elapsed_accumulated / iterations));
  return(0);
}
