/*
 * scratch from source code of gcc-4.8.2
 */


#include <stdio.h>
#include "cpuid.h"

enum processor_type
{
  PROCESSOR_I386 = 0,			/* 80386 */
  PROCESSOR_I486,			/* 80486DX, 80486SX, 80486DX[24] */
  PROCESSOR_PENTIUM,
  PROCESSOR_PENTIUMPRO,
  PROCESSOR_GEODE,
  PROCESSOR_K6,
  PROCESSOR_ATHLON,
  PROCESSOR_PENTIUM4,
  PROCESSOR_K8,
  PROCESSOR_NOCONA,
  PROCESSOR_CORE2,
  PROCESSOR_COREI7,
  PROCESSOR_HASWELL,
  PROCESSOR_GENERIC32,
  PROCESSOR_GENERIC64,
  PROCESSOR_AMDFAM10,
  PROCESSOR_BDVER1,
  PROCESSOR_BDVER2,
  PROCESSOR_BDVER3,
  PROCESSOR_BTVER1,
  PROCESSOR_BTVER2,
  PROCESSOR_ATOM,
  PROCESSOR_max
};







/* This will be called by the spec parser in gcc.c when it sees
   a %:local_cpu_detect(args) construct.  Currently it will be called
   with either "arch" or "tune" as argument depending on if -march=native
   or -mtune=native is to be substituted.

   It returns a string containing new command line parameters to be
   put at the place of the above two options, depending on what CPU
   this is executed.  E.g. "-march=k8" on an AMD64 machine
   for -march=native.

   ARGC and ARGV are set depending on the actual arguments given
   in the spec.  */

const char *host_detect_local_cpu()
{
  enum processor_type processor = PROCESSOR_I386;
  const char *cpu = "i386";

  const char *cache = "";
  const char *options = "";

  unsigned int eax, ebx, ecx, edx;

  unsigned int max_level, ext_level;

  unsigned int vendor;
  unsigned int model, family;

  unsigned int has_sse3, has_ssse3, has_cmpxchg16b;
  unsigned int has_cmpxchg8b, has_cmov, has_mmx, has_sse, has_sse2;

  /* Extended features */
  unsigned int has_lahf_lm = 0, has_sse4a = 0;
  unsigned int has_longmode = 0, has_3dnowp = 0, has_3dnow = 0;
  unsigned int has_movbe = 0, has_sse4_1 = 0, has_sse4_2 = 0;
  unsigned int has_popcnt = 0, has_aes = 0, has_avx = 0, has_avx2 = 0;
  unsigned int has_pclmul = 0, has_abm = 0, has_lwp = 0;
  unsigned int has_fma = 0, has_fma4 = 0, has_xop = 0;
  unsigned int has_bmi = 0, has_bmi2 = 0, has_tbm = 0, has_lzcnt = 0;
  unsigned int has_hle = 0, has_rtm = 0;
  unsigned int has_rdrnd = 0, has_f16c = 0, has_fsgsbase = 0;
  unsigned int has_rdseed = 0, has_prfchw = 0, has_adx = 0;
  unsigned int has_osxsave = 0, has_fxsr = 0, has_xsave = 0, has_xsaveopt = 0;

  int arch=1;

  unsigned int l2sizekb = 0;




  max_level = __get_cpuid_max (0, &vendor);
  if (max_level < 1)
	goto done;

  __cpuid (1, eax, ebx, ecx, edx);

  model = (eax >> 4) & 0x0f;
  family = (eax >> 8) & 0x0f;
  if (vendor == signature_INTEL_ebx)
	{
	  unsigned int extended_model, extended_family;

	  extended_model = (eax >> 12) & 0xf0;
	  extended_family = (eax >> 20) & 0xff;
	  if (family == 0x0f)
	{
	  family += extended_family;
	  model += extended_model;
	}
	  else if (family == 0x06)
	model += extended_model;
	}

  has_sse3 = ecx & bit_SSE3;
  has_ssse3 = ecx & bit_SSSE3;
  has_sse4_1 = ecx & bit_SSE4_1;
  has_sse4_2 = ecx & bit_SSE4_2;
  has_avx = ecx & bit_AVX;
  has_osxsave = ecx & bit_OSXSAVE;
  has_cmpxchg16b = ecx & bit_CMPXCHG16B;
  has_movbe = ecx & bit_MOVBE;
  has_popcnt = ecx & bit_POPCNT;
  has_aes = ecx & bit_AES;
  has_pclmul = ecx & bit_PCLMUL;
  has_fma = ecx & bit_FMA;
  has_f16c = ecx & bit_F16C;
  has_rdrnd = ecx & bit_RDRND;
  has_xsave = ecx & bit_XSAVE;

  has_cmpxchg8b = edx & bit_CMPXCHG8B;
  has_cmov = edx & bit_CMOV;
  has_mmx = edx & bit_MMX;
  has_fxsr = edx & bit_FXSAVE;
  has_sse = edx & bit_SSE;
  has_sse2 = edx & bit_SSE2;

  if (max_level >= 7)
	{
	  __cpuid_count (7, 0, eax, ebx, ecx, edx);

	  has_bmi = ebx & bit_BMI;
	  has_hle = ebx & bit_HLE;
	  has_rtm = ebx & bit_RTM;
	  has_avx2 = ebx & bit_AVX2;
	  has_bmi2 = ebx & bit_BMI2;
	  has_fsgsbase = ebx & bit_FSGSBASE;
	  has_rdseed = ebx & bit_RDSEED;
	  has_adx = ebx & bit_ADX;
	}

  if (max_level >= 13)
	{
	  __cpuid_count (13, 1, eax, ebx, ecx, edx);

	  has_xsaveopt = eax & bit_XSAVEOPT;
	}

  /* Get XCR_XFEATURE_ENABLED_MASK register with xgetbv.  */
#define XCR_XFEATURE_ENABLED_MASK	0x0
#define XSTATE_FP			0x1
#define XSTATE_SSE			0x2
#define XSTATE_YMM			0x4
  if (has_osxsave)
	asm (".byte 0x0f; .byte 0x01; .byte 0xd0"
	 : "=a" (eax), "=d" (edx)
	 : "c" (XCR_XFEATURE_ENABLED_MASK));

  /* Check if SSE and YMM states are supported.  */
  if (!has_osxsave
	  || (eax & (XSTATE_SSE | XSTATE_YMM)) != (XSTATE_SSE | XSTATE_YMM))
	{
	  has_avx = 0;
	  has_avx2 = 0;
	  has_fma = 0;
	  has_fma4 = 0;
	  has_xop = 0;
	  has_xsave = 0;
	  has_xsaveopt = 0;
	}

  /* Check cpuid level of extended features.  */
  __cpuid (0x80000000, ext_level, ebx, ecx, edx);

  if (ext_level > 0x80000000)
	{
	  __cpuid (0x80000001, eax, ebx, ecx, edx);

	  has_lahf_lm = ecx & bit_LAHF_LM;
	  has_sse4a = ecx & bit_SSE4a;
	  has_abm = ecx & bit_ABM;
	  has_lwp = ecx & bit_LWP;
	  has_fma4 = ecx & bit_FMA4;
	  has_xop = ecx & bit_XOP;
	  has_tbm = ecx & bit_TBM;
	  has_lzcnt = ecx & bit_LZCNT;
	  has_prfchw = ecx & bit_PRFCHW;

	  has_longmode = edx & bit_LM;
	  has_3dnowp = edx & bit_3DNOWP;
	  has_3dnow = edx & bit_3DNOW;
	}

	

  if (vendor == signature_AMD_ebx)
	{
	  unsigned int name;

	  /* Detect geode processor by its processor signature.  */
	  if (ext_level > 0x80000001)
	__cpuid (0x80000002, name, ebx, ecx, edx);
	  else
	name = 0;

	  if (name == signature_NSC_ebx)
	processor = PROCESSOR_GEODE;
	  else if (has_movbe)
	processor = PROCESSOR_BTVER2;
	  else if (has_xsaveopt)
		processor = PROCESSOR_BDVER3;
	  else if (has_bmi)
		processor = PROCESSOR_BDVER2;
	  else if (has_xop)
	processor = PROCESSOR_BDVER1;
	  else if (has_sse4a && has_ssse3)
		processor = PROCESSOR_BTVER1;
	  else if (has_sse4a)
	processor = PROCESSOR_AMDFAM10;
	  else if (has_sse2 || has_longmode)
	processor = PROCESSOR_K8;
	  else if (has_3dnowp && family == 6)
	processor = PROCESSOR_ATHLON;
	  else if (has_mmx)
	processor = PROCESSOR_K6;
	  else
	processor = PROCESSOR_PENTIUM;
	}
  else if (vendor == signature_CENTAUR_ebx)
	{
	  if (arch)
	{
	  switch (family)
		{
		case 6:
		  if (model > 9)
		/* Use the default detection procedure.  */
		processor = PROCESSOR_GENERIC32;
		  else if (model == 9)
		cpu = "c3-2";
		  else if (model >= 6)
		cpu = "c3";
		  else
		processor = PROCESSOR_GENERIC32;
		  break;
		case 5:
		  if (has_3dnow)
		cpu = "winchip2";
		  else if (has_mmx)
		cpu = "winchip2-c6";
		  else
		processor = PROCESSOR_GENERIC32;
		  break;
		default:
		  /* We have no idea.  */
		  processor = PROCESSOR_GENERIC32;
		}
	}
	}
  else
	{
	  switch (family)
	{
	case 4:
	  processor = PROCESSOR_I486;
	  break;
	case 5:
	  processor = PROCESSOR_PENTIUM;
	  break;
	case 6:
	  processor = PROCESSOR_PENTIUMPRO;
	  break;
	case 15:
	  processor = PROCESSOR_PENTIUM4;
	  break;
	default:
	  /* We have no idea.  */
	  processor = PROCESSOR_GENERIC32;
	}
	}

  switch (processor)
	{
	case PROCESSOR_I386:
	  /* Default.  */
	  break;
	case PROCESSOR_I486:
	  cpu = "i486";
	  break;
	case PROCESSOR_PENTIUM:
	  if (arch && has_mmx)
	cpu = "pentium-mmx";
	  else
	cpu = "pentium";
	  break;
	case PROCESSOR_PENTIUMPRO:
	  switch (model)
	{
	case 0x1c:
	case 0x26:
	  /* Atom.	*/
	  cpu = "atom";
	  break;
	case 0x0f:
	  /* Merom.  */
	case 0x17:
	case 0x1d:
	  /* Penryn.  */
	  cpu = "core2";
	  break;
	case 0x1a:
	case 0x1e:
	case 0x1f:
	case 0x2e:
	  /* Nehalem.  */
	case 0x25:
	case 0x2c:
	case 0x2f:
	  /* Westmere.	*/
	  cpu = "corei7";
	  break;
	case 0x2a:
	case 0x2d:
	  /* Sandy Bridge.	*/
	  cpu = "corei7-avx";
	  break;
	case 0x3a:
	case 0x3e:
	  /* Ivy Bridge.  */
	  cpu = "core-avx-i";
	  break;
	case 0x3c:
	case 0x45:
	case 0x46:
	  /* Haswell.  */
	  cpu = "core-avx2";
	  break;
	default:
	  if (arch)
		{
		  /* This is unknown family 0x6 CPU.  */
		  if (has_avx2)
		/* Assume Haswell.	*/
		cpu = "core-avx2";
		  else if (has_avx)
		/* Assume Sandy Bridge.  */
		cpu = "corei7-avx";
		  else if (has_sse4_2)
		/* Assume Core i7.	*/
		cpu = "corei7";
		  else if (has_ssse3)
		{
		  if (has_movbe)
			/* Assume Atom.  */
			cpu = "atom";
		  else
			/* Assume Core 2.  */
			cpu = "core2";
		}
		  else if (has_sse3)
		/* It is Core Duo.	*/
		cpu = "pentium-m";
		  else if (has_sse2)
		/* It is Pentium M.  */
		cpu = "pentium-m";
		  else if (has_sse)
		/* It is Pentium III.  */
		cpu = "pentium3";
		  else if (has_mmx)
		/* It is Pentium II.  */
		cpu = "pentium2";
		  else
		/* Default to Pentium Pro.	*/
		cpu = "pentiumpro";
		}
	  else
		/* For -mtune, we default to -mtune=generic.  */
		cpu = "generic";
	  break;
	}
	  break;
	case PROCESSOR_PENTIUM4:
	  if (has_sse3)
	{
	  if (has_longmode)
		cpu = "nocona";
	  else
		cpu = "prescott";
	}
	  else
	cpu = "pentium4";
	  break;
	case PROCESSOR_GEODE:
	  cpu = "geode";
	  break;
	case PROCESSOR_K6:
	  if (arch && has_3dnow)
	cpu = "k6-3";
	  else
	cpu = "k6";
	  break;
	case PROCESSOR_ATHLON:
	  if (arch && has_sse)
	cpu = "athlon-4";
	  else
	cpu = "athlon";
	  break;
	case PROCESSOR_K8:
	  if (arch && has_sse3)
	cpu = "k8-sse3";
	  else
	cpu = "k8";
	  break;
	case PROCESSOR_AMDFAM10:
	  cpu = "amdfam10";
	  break;
	case PROCESSOR_BDVER1:
	  cpu = "bdver1";
	  break;
	case PROCESSOR_BDVER2:
	  cpu = "bdver2";
	  break;
	case PROCESSOR_BDVER3:
	  cpu = "bdver3";
	  break;
	case PROCESSOR_BTVER1:
	  cpu = "btver1";
	  break;
	case PROCESSOR_BTVER2:
	  cpu = "btver2";
	  break;

	default:
	  /* Use something reasonable.	*/
	  if (arch)
	{
	  if (has_ssse3)
		cpu = "core2";
	  else if (has_sse3)
		{
		  if (has_longmode)
		cpu = "nocona";
		  else
		cpu = "prescott";
		}
	  else if (has_sse2)
		cpu = "pentium4";
	  else if (has_cmov)
		cpu = "pentiumpro";
	  else if (has_mmx)
		cpu = "pentium-mmx";
	  else if (has_cmpxchg8b)
		cpu = "pentium";
	}
	  else
	cpu = "generic";
	}

  if (arch)
	{
	  const char *cx16 = has_cmpxchg16b ? " -mcx16" : " -mno-cx16";
	  const char *sahf = has_lahf_lm ? " -msahf" : " -mno-sahf";
	  const char *movbe = has_movbe ? " -mmovbe" : " -mno-movbe";
	  const char *ase = has_aes ? " -maes" : " -mno-aes";
	  const char *pclmul = has_pclmul ? " -mpclmul" : " -mno-pclmul";
	  const char *popcnt = has_popcnt ? " -mpopcnt" : " -mno-popcnt";
	  const char *abm = has_abm ? " -mabm" : " -mno-abm";
	  const char *lwp = has_lwp ? " -mlwp" : " -mno-lwp";
	  const char *fma = has_fma ? " -mfma" : " -mno-fma";
	  const char *fma4 = has_fma4 ? " -mfma4" : " -mno-fma4";
	  const char *xop = has_xop ? " -mxop" : " -mno-xop";
	  const char *bmi = has_bmi ? " -mbmi" : " -mno-bmi";
	  const char *bmi2 = has_bmi2 ? " -mbmi2" : " -mno-bmi2";
	  const char *tbm = has_tbm ? " -mtbm" : " -mno-tbm";
	  const char *avx = has_avx ? " -mavx" : " -mno-avx";
	  const char *avx2 = has_avx2 ? " -mavx2" : " -mno-avx2";
	  const char *sse4_2 = has_sse4_2 ? " -msse4.2" : " -mno-sse4.2";
	  const char *sse4_1 = has_sse4_1 ? " -msse4.1" : " -mno-sse4.1";
	  const char *lzcnt = has_lzcnt ? " -mlzcnt" : " -mno-lzcnt";
	  const char *hle = has_hle ? " -mhle" : " -mno-hle";
	  const char *rtm = has_rtm ? " -mrtm" : " -mno-rtm";
	  const char *rdrnd = has_rdrnd ? " -mrdrnd" : " -mno-rdrnd";
	  const char *f16c = has_f16c ? " -mf16c" : " -mno-f16c";
	  const char *fsgsbase = has_fsgsbase ? " -mfsgsbase" : " -mno-fsgsbase";
	  const char *rdseed = has_rdseed ? " -mrdseed" : " -mno-rdseed";
	  const char *prfchw = has_prfchw ? " -mprfchw" : " -mno-prfchw";
	  const char *adx = has_adx ? " -madx" : " -mno-adx";
	  const char *fxsr = has_fxsr ? " -mfxsr" : " -mno-fxsr";
	  const char *xsave = has_xsave ? " -mxsave" : " -mno-xsave";
	  const char *xsaveopt = has_xsaveopt ? " -mxsaveopt" : " -mno-xsaveopt";

	}

done:
  return cpu;
}


int main(int argc, char *argv[])
{
    printf("%s\n", host_detect_local_cpu());
    return 0;
}

