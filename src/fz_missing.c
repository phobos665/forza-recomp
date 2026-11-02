/*
 * fz_missing.c - guest functions the lift does not contain.
 *
 * Translated by hand from the XBE in the generated code's own form (the same
 * macros, the same call and return conventions), and reached through
 * recomp_lookup_manual. Named fz_sub_ so a later lift that does contain the
 * function cannot collide with it; the manual lookup runs first either way.
 */
#define RECOMP_GENERATED_CODE
#include "recomp/gen/recomp_funcs.h"
#include <math.h>

void fz_sub_0019D240(void);
void fz_crt_tanh_0x002FE3D9(void);
void fz_sub_0027F450(void);
void fz_sub_0001AEC0(void);

/*
 * 0x0019D240 - 0x0019D35A: the post-race step "Updating Scoreboards".
 *
 * A state callback (thiscall, one argument, ret 4) that the race-end state
 * machine at 0x00096C30 calls through this+0x1C: it waits until both
 * leaderboard writers (0x000A6A90 and 0x00331220, status 100 = done) are
 * finished, shows the result text and hands over to the next step,
 * 0x0019D160, by storing it in this+0x1C. Nothing reaches it by a direct call,
 * so the function analysis never found it and the lift has no body for it:
 * every call was refused ("[ICALL] unresolved call target 0x0019D240"), the
 * state never advanced and the game stayed on "Updating Scoreboards".
 *
 *   19d240 mov eax,[esp+4]        19d244 sub esp,0x20     19d247 test eax,eax
 *   19d249 push edi               19d24a mov edi,ecx      19d24c jne 19d353
 *   19d252 call a6a90 ; [eax]->vt[0x2c]() == 100 ?  else 19d353
 *   19d267 call 331220; [eax]->vt[0x2c]() == 100 ?  else 19d353
 *   19d27c call a6a90 ; [eax]->vt[0x28]() == 0 ?    else 19d353
 *   19d290 call 331220; [eax]->vt[0x4c](1)
 *   19d29e call 331220; [eax]->vt[0x30]() == 100 ?  else 19d353
 *   19d2b3 string from 0x4b07a0 -> (edi+0x1c18)->129bf0, temp destroyed
 *   19d2e2 call 331220; [eax]->vt[0x34]() == 100 ?  else 19d352
 *   19d2f3 (edi+0x1c18)->97610()
 *   19d2fa call 331220; [eax]->vt[0x1c]() as bool ?  else 19d349
 *   19d30a string from 0x4b1734 -> (edi+0x23bc)->275b00, temp destroyed
 *   19d336 pop esi; [edi+0x1c] = 0x19d160; [edi+0x18] = 1; pop edi;
 *          add esp,0x20; ret 4
 *   19d349 push 1; ecx = edi; call 19d050
 *   19d352 pop esi
 *   19d353 pop edi; add esp,0x20; ret 4
 */
void fz_sub_0019D240(void)
{
loc_0019D240: ;
    eax = MEM32(esp + 4);
    esp = esp - 0x20;
    PUSH32(esp, edi);
    edi = ecx;
    if (eax != 0) goto loc_0019D353;

loc_0019D252: ;
    PUSH32(esp, 0x0019D257u); RECOMP_ABI_CALL_POP(0x000A6A90u, sub_000A6A90, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x2C); PUSH32(esp, 0x0019D25Eu); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D25Bu); }
    }
    if (eax != 0x64) goto loc_0019D353;

loc_0019D267: ;
    PUSH32(esp, 0x0019D26Cu); RECOMP_ABI_CALL_POP(0x00331220u, sub_00331220, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x2C); PUSH32(esp, 0x0019D273u); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D270u); }
    }
    if (eax != 0x64) goto loc_0019D353;

loc_0019D27C: ;
    PUSH32(esp, 0x0019D281u); RECOMP_ABI_CALL_POP(0x000A6A90u, sub_000A6A90, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x28); PUSH32(esp, 0x0019D288u); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D285u); }
    }
    if (eax != 0) goto loc_0019D353;

loc_0019D290: ;
    PUSH32(esp, 0x0019D295u); RECOMP_ABI_CALL_POP(0x00331220u, sub_00331220, 0u);
    edx = MEM32(eax);
    { uint32_t _icall_esp = g_esp;
    PUSH32(esp, 1);
    ecx = eax;
    { uint32_t _icall_target = MEM32(edx + 0x4C); PUSH32(esp, 0x0019D29Eu); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D29Bu); }
    }

loc_0019D29E: ;
    PUSH32(esp, 0x0019D2A3u); RECOMP_ABI_CALL_POP(0x00331220u, sub_00331220, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x30); PUSH32(esp, 0x0019D2AAu); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D2A7u); }
    }
    if (eax != 0x64) goto loc_0019D353;

loc_0019D2B3: ;
    eax = 0;
    SET_LO16(eax, MEM16(0x4B07A0));
    PUSH32(esp, esi);
    ecx = esp + 8;
    esi = edi + 0x1C18;
    PUSH32(esp, eax);
    PUSH32(esp, 0x5BAD28);
    PUSH32(esp, 0x0019D2D1u); RECOMP_ABI_CALL_POP(0x00275380u, sub_00275380, 8u);

loc_0019D2D1: ;
    PUSH32(esp, eax);
    ecx = esi;
    PUSH32(esp, 0x0019D2D9u); RECOMP_ABI_CALL_POP(0x00129BF0u, sub_00129BF0, 4u);

loc_0019D2D9: ;
    ecx = esp + 8;
    PUSH32(esp, 0x0019D2E2u); RECOMP_ABI_CALL_POP(0x002753B0u, sub_002753B0, 0u);

loc_0019D2E2: ;
    PUSH32(esp, 0x0019D2E7u); RECOMP_ABI_CALL_POP(0x00331220u, sub_00331220, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x34); PUSH32(esp, 0x0019D2EEu); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D2EBu); }
    }
    if (eax != 0x64) goto loc_0019D352;

loc_0019D2F3: ;
    ecx = esi;
    PUSH32(esp, 0x0019D2FAu); RECOMP_ABI_CALL_POP(0x00097610u, sub_00097610, 0u);

loc_0019D2FA: ;
    PUSH32(esp, 0x0019D2FFu); RECOMP_ABI_CALL_POP(0x00331220u, sub_00331220, 0u);
    edx = MEM32(eax);
    ecx = eax;
    { uint32_t _icall_esp = g_esp;
    { uint32_t _icall_target = MEM32(edx + 0x1C); PUSH32(esp, 0x0019D306u); RECOMP_ICALL_SAFE_AT(_icall_target, _icall_esp, 0x0019D303u); }
    }
    if (LO8(eax) == 0) goto loc_0019D349;

loc_0019D30A: ;
    eax = 0;
    SET_LO16(eax, MEM16(0x4B1734));
    ecx = esp + 0x18;
    PUSH32(esp, eax);
    PUSH32(esp, 0x5BAD28);
    PUSH32(esp, 0x0019D321u); RECOMP_ABI_CALL_POP(0x00275380u, sub_00275380, 8u);

loc_0019D321: ;
    PUSH32(esp, eax);
    ecx = edi + 0x23BC;
    PUSH32(esp, 0x0019D32Du); RECOMP_ABI_CALL_POP(0x00275B00u, sub_00275B00, 4u);

loc_0019D32D: ;
    ecx = esp + 0x18;
    PUSH32(esp, 0x0019D336u); RECOMP_ABI_CALL_POP(0x002753B0u, sub_002753B0, 0u);

loc_0019D336: ;
    POP32(esp, esi);
    MEM32(edi + 0x1C) = 0x19D160;
    MEM8(edi + 0x18) = 1;
    POP32(esp, edi);
    esp = esp + 0x20;
    esp += 8; return; /* ret 4 */

loc_0019D349: ;
    PUSH32(esp, 1);
    ecx = edi;
    PUSH32(esp, 0x0019D352u); RECOMP_ABI_CALL_POP(0x0019D050u, sub_0019D050, 4u);

loc_0019D352: ;
    POP32(esp, esi);

loc_0019D353: ;
    POP32(esp, edi);
    esp = esp + 0x20;
    esp += 8; return; /* ret 4 */
}

/*
 * 0x002FE3D9: the C runtime's tanh core (MSVC _CItanh), entered by a tail
 * jump from the x87 argument dispatcher 0x002FC8B8 (_trandisp1), which jumps
 * through a table at 0x005748BC by the class FXAM gives the argument. This
 * entry is the one for an ordinary finite number. Like the function above it
 * is reached only that way, so the lift has no body for it, and every call
 * was refused ("[ICALL] unresolved jump target 0x002FE3D9", over 10,000 times
 * a race): the refusal pops the return address and leaves the argument on
 * the x87 stack, so tanh(x) came back as x.
 *
 *   2fe3d9 fld st0 / fabs / fld tbyte [0x574a58] (19.0615...) / fcompp
 *          |x| beyond it: the saturated path (2fe45a)
 *          else e = 2^(x*log2 e) by the shared helpers 2fe468/2fd6e2,
 *          (e - 1/e) / (e + 1/e) through 2fe499/2fe4ab, then ret
 *
 * Done here with the host's tanh, which also saturates to +-1 out there,
 * rounded as the guest's precision control rounds a result (the dispatcher
 * has just loaded 0x133F, 64-bit precision). It returns as the guest code
 * does: result in st(0), `ret` popping the address the dispatcher's caller
 * pushed (0x002FE519 for the call that showed up). The guest code also
 * writes an FSTSW scratch word in the caller's frame and, on the saturated
 * path only, an error class at [ebp-0x90]; neither is read for an ordinary
 * result.
 */
void fz_crt_tanh_0x002FE3D9(void)
{
    double *st0 = &g_fp_stack[g_fp_top & 7u];
    *st0 = RECOMP_FP_PC(tanh(*st0));
    g_esp += 4;   /* ret */
}

/*
 * 0x0027F450 - 0x0027F478: the background loader's thread.
 *
 * Started once at boot through PsCreateSystemThreadEx (ctx1 = 0x0027F450),
 * so nothing reaches it by a direct call and the lift has no body for it:
 * "[ICALL] unresolved call target 0x0027F450" at start-up, and the thread
 * ended at once. Its queue (the object at 0x5E5CD8) is where everything the
 * game loads in the background is asked for. The race loads its cars on the
 * main thread and never noticed; the car selection asks the queue for the
 * car on the turntable and waits for it, every frame, forever -- the screen
 * showed no car at all.
 *
 *   27f450 call 10b390
 *   27f455 mov ecx,[5e5cd0]     27f45b call 27f190   ; run what is queued
 *   27f460 call 110310
 *   27f465 mov eax,[5e5cd4]     push -1; push eax
 *   27f46d call 264ea5          ; WaitForSingleObject(queue semaphore, INFINITE)
 *   27f472 call 110310
 *   27f477 jmp 27f455
 */
void fz_sub_0027F450(void)
{
    PUSH32(esp, 0x0027F455u); RECOMP_ABI_CALL(0x0010B390u, sub_0010B390);
loc_0027F455: ;
    RECOMP_BACKEDGE();
    ecx = MEM32(0x5E5CD0);
    PUSH32(esp, 0x0027F460u); RECOMP_ABI_CALL(0x0027F190u, sub_0027F190);
    PUSH32(esp, 0x0027F465u); RECOMP_ABI_CALL(0x00110310u, sub_00110310);
    eax = MEM32(0x5E5CD4);
    PUSH32(esp, 0xFFFFFFFFu);
    PUSH32(esp, eax);
    PUSH32(esp, 0x0027F472u); RECOMP_ABI_CALL(0x00264EA5u, sub_00264EA5);
    PUSH32(esp, 0x0027F477u); RECOMP_ABI_CALL(0x00110310u, sub_00110310);
    goto loc_0027F455;
}

/*
 * 0x0001AEC0: the audio manager's update thread (manager at 0x00575418,
 * passed as the thread's argument).
 *
 *   1aec0 push ebx; mov ebx,[esp+8]
 *   1aec5 mov al,[ebx+0x315C]; test al,al; je 1af98   ; not running: leave
 *   ...   loop: sound update 0x1A9F0, music update 0x2E770 (which feeds the
 *         menu music stream), until [ebx+0x315C] is cleared
 *
 * The creator (0x0001AFC0) starts the thread and only afterwards sets
 * [manager+0x315C] = 1. On the console the new thread does not run before
 * its creator gives up the processor, so it always saw 1. Here it starts at
 * once on its own core, often still saw 0 and returned at once: the log shows
 * a PsTerminateSystemThread straight after the spawn. With no update thread
 * nothing ever decoded the menu music (UIMusic.wmapack): its stream was made
 * but never got a packet ("streams used 0" for the whole of the menus).
 *
 * So wait here, bounded, for the creator to set the flag, then run the lifted
 * thread as it is. The guest stack is untouched: the lifted body finds its
 * argument and return address where the thread start put them.
 */
__declspec(dllimport) void __stdcall Sleep(unsigned long ms);

void fz_sub_0001AEC0(void)
{
    uint32_t mgr = MEM32(esp + 4);
    for (int i = 0; i < 2000 && mgr && MEM8(mgr + 0x315Cu) == 0; i++)
        Sleep(1);
    sub_0001AEC0();
}


/* Forza's stream setters already run on the guest, but the older generated
 * HLE bindings do not notify the native stream model. Keep those bodies and
 * their HRESULT/stack behaviour; forward successful changes to the host. */
int recomp_hle_audio_native(void);
void hle_dsound_stream_format_changed(uint32_t object, uint32_t wfx);
void hle_IDirectSoundStream_SetPitch(void);
void sub_003F4753_stream_audio_original(void);
void sub_003F29C7_stream_audio_original(void);

void sub_003F4753(void)
{
    uint32_t object = MEM32(esp + 4u), wfx = MEM32(esp + 8u);
    sub_003F4753_stream_audio_original();
    if (recomp_hle_audio_native() && (int32_t)eax >= 0)
        hle_dsound_stream_format_changed(object, wfx);
}

void sub_003F29C7(void)
{
    uint32_t before = esp, after, result;
    sub_003F29C7_stream_audio_original();
    after = esp;
    result = eax;
    if (recomp_hle_audio_native() && (int32_t)result >= 0) {
        esp = before;
        hle_IDirectSoundStream_SetPitch();
        esp = after;
        eax = result;
    }
}
