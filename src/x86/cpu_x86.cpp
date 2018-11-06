/* cpu_x86.cpp
 * 
 * Author           : Alexander J. Yee
 * Date Created     : 04/12/2014
 * Last Modified    : 04/12/2014
 * 
 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Dependencies
#include <cstring>
#include "cpu_x86.h"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#   if _WIN32
#       include "cpu_x86_Windows.ipp"
#   elif defined(__GNUC__) || defined(__clang__)
#       include "cpu_x86_Linux.ipp"
#   else
#       error "No cpuid intrinsic defined for compiler."
#   endif
#else
#   error "No cpuid intrinsic defined for processor architecture."
#endif

namespace FeatureDetector{
    using std::ostream;
    using std::endl;
    using std::memcpy;
    using std::memset;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void cpu_x86::print(ostream& os, const char* label, bool yes){
    os << label;
    os << (yes ? "Yes" : "No") << endl;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
cpu_x86::cpu_x86(){
    memset(this, 0, sizeof(*this));
}
bool cpu_x86::detect_OS_AVX(){
    //  Copied from: http://stackoverflow.com/a/22521619/922184

    bool avxSupported = false;

    int cpuInfo[4];
    cpuid(cpuInfo, 1);

    bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    bool cpuAVXSuport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
        uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        avxSupported = (xcrFeatureMask & 0x6) == 0x6;
    }

    return avxSupported;
}
bool cpu_x86::detect_OS_AVX512(){
    if (!detect_OS_AVX())
        return false;

    uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
    return (xcrFeatureMask & 0xe6) == 0xe6;
}
std::string cpu_x86::get_vendor_string(){
    int32_t CPUInfo[4];
    char name[13];

    cpuid(CPUInfo, 0);
    memcpy(name + 0, &CPUInfo[1], 4);
    memcpy(name + 4, &CPUInfo[3], 4);
    memcpy(name + 8, &CPUInfo[2], 4);
    name[12] = '\0';

    return name;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void cpu_x86::detect_host(){
    //  OS Features
    OS_x64 = detect_OS_x64();
    OS_AVX = detect_OS_AVX();
    OS_AVX512 = detect_OS_AVX512();

    //  Vendor
    std::string vendor(get_vendor_string());
    if (vendor == "GenuineIntel"){
        Vendor_Intel = true;
    }else if (vendor == "AuthenticAMD"){
        Vendor_AMD = true;
    }

    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    uint32_t nExIds = info[0];

    //  Detect Features
    if (nIds >= 0x00000001){
        cpuid(info, 0x00000001);
        HW_MMX    = (info[3] & ((int)1 << 23)) != 0;
        HW_SSE    = (info[3] & ((int)1 << 25)) != 0;
        HW_SSE2   = (info[3] & ((int)1 << 26)) != 0;
        HW_SSE3   = (info[2] & ((int)1 <<  0)) != 0;

        HW_SSSE3  = (info[2] & ((int)1 <<  9)) != 0;
        HW_SSE41  = (info[2] & ((int)1 << 19)) != 0;
        HW_SSE42  = (info[2] & ((int)1 << 20)) != 0;
        HW_AES    = (info[2] & ((int)1 << 25)) != 0;

        HW_AVX    = (info[2] & ((int)1 << 28)) != 0;
        HW_FMA3   = (info[2] & ((int)1 << 12)) != 0;

        HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
    }
    if (nIds >= 0x00000007){
        cpuid(info, 0x00000007);
        HW_AVX2         = (info[1] & ((int)1 <<  5)) != 0;

        HW_BMI1         = (info[1] & ((int)1 <<  3)) != 0;
        HW_BMI2         = (info[1] & ((int)1 <<  8)) != 0;
        HW_ADX          = (info[1] & ((int)1 << 19)) != 0;
        HW_MPX          = (info[1] & ((int)1 << 14)) != 0;
        HW_SHA          = (info[1] & ((int)1 << 29)) != 0;
        HW_PREFETCHWT1  = (info[2] & ((int)1 <<  0)) != 0;

        HW_AVX512_F     = (info[1] & ((int)1 << 16)) != 0;
        HW_AVX512_CD    = (info[1] & ((int)1 << 28)) != 0;
        HW_AVX512_PF    = (info[1] & ((int)1 << 26)) != 0;
        HW_AVX512_ER    = (info[1] & ((int)1 << 27)) != 0;
        HW_AVX512_VL    = (info[1] & ((int)1 << 31)) != 0;
        HW_AVX512_BW    = (info[1] & ((int)1 << 30)) != 0;
        HW_AVX512_DQ    = (info[1] & ((int)1 << 17)) != 0;
        HW_AVX512_IFMA  = (info[1] & ((int)1 << 21)) != 0;
        HW_AVX512_VBMI  = (info[2] & ((int)1 <<  1)) != 0;
    }
    if (nExIds >= 0x80000001){
        cpuid(info, 0x80000001);
        HW_x64   = (info[3] & ((int)1 << 29)) != 0;
        HW_ABM   = (info[2] & ((int)1 <<  5)) != 0;
        HW_SSE4a = (info[2] & ((int)1 <<  6)) != 0;
        HW_FMA4  = (info[2] & ((int)1 << 16)) != 0;
        HW_XOP   = (info[2] & ((int)1 << 11)) != 0;
    }
}
void cpu_x86::print(ostream& os) const{
    os << "CPU Vendor:" << endl;
    print(os, "    AMD         = ", Vendor_AMD);
    print(os, "    Intel       = ", Vendor_Intel);
    os << endl;

    os << "OS Features:" << endl;
#ifdef _WIN32
    print(os, "    64-bit      = ", OS_x64);
#endif
    print(os, "    OS AVX      = ", OS_AVX);
    print(os, "    OS AVX512   = ", OS_AVX512);
    os << endl;

    os << "Hardware Features:" << endl;
    print(os, "    MMX         = ", HW_MMX);
    print(os, "    x64         = ", HW_x64);
    print(os, "    ABM         = ", HW_ABM);
    print(os, "    RDRAND      = ", HW_RDRAND);
    print(os, "    BMI1        = ", HW_BMI1);
    print(os, "    BMI2        = ", HW_BMI2);
    print(os, "    ADX         = ", HW_ADX);
    print(os, "    MPX         = ", HW_MPX);
    print(os, "    PREFETCHWT1 = ", HW_PREFETCHWT1);
    os << endl;

    os << "SIMD: 128-bit" << endl;
    print(os, "    SSE         = ", HW_SSE);
    print(os, "    SSE2        = ", HW_SSE2);
    print(os, "    SSE3        = ", HW_SSE3);
    print(os, "    SSSE3       = ", HW_SSSE3);
    print(os, "    SSE4a       = ", HW_SSE4a);
    print(os, "    SSE4.1      = ", HW_SSE41);
    print(os, "    SSE4.2      = ", HW_SSE42);
    print(os, "    AES-NI      = ", HW_AES);
    print(os, "    SHA         = ", HW_SHA);
    os << endl;

    os << "SIMD: 256-bit" << endl;
    print(os, "    AVX         = ", HW_AVX);
    print(os, "    XOP         = ", HW_XOP);
    print(os, "    FMA3        = ", HW_FMA3);
    print(os, "    FMA4        = ", HW_FMA4);
    print(os, "    AVX2        = ", HW_AVX2);
    os << endl;

    os << "SIMD: 512-bit" << endl;
    print(os, "    AVX512-F    = ", HW_AVX512_F);
    print(os, "    AVX512-CD   = ", HW_AVX512_CD);
    print(os, "    AVX512-PF   = ", HW_AVX512_PF);
    print(os, "    AVX512-ER   = ", HW_AVX512_ER);
    print(os, "    AVX512-VL   = ", HW_AVX512_VL);
    print(os, "    AVX512-BW   = ", HW_AVX512_BW);
    print(os, "    AVX512-DQ   = ", HW_AVX512_DQ);
    print(os, "    AVX512-IFMA = ", HW_AVX512_IFMA);
    print(os, "    AVX512-VBMI = ", HW_AVX512_VBMI);
    os << endl;

    os << "Summary:" << endl;
    print(os, "    Safe to use AVX:     ", HW_AVX && OS_AVX);
    print(os, "    Safe to use AVX512:  ", HW_AVX512_F && OS_AVX512);
    os << endl;
}
void cpu_x86::print_host(){
    cpu_x86 features;
    features.detect_host();
    features.print();
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
}
