import subprocess
import textwrap


def test_led_matrix_firmware_commands_parse_brightness_effect_mode_and_format_ha_errors(tmp_path):
    source = tmp_path / "firmware_commands_test.cpp"
    binary = tmp_path / "firmware_commands_test"
    source.write_text(
        textwrap.dedent(
            r'''
            #include <cassert>
            #include <cstdint>
            #include <cstring>
            #include <string>
            #include <LedMatrixFirmwareCommands.h>

            int main() {
                uint8_t brightness = 0;
                assert(parseLedMatrixBrightnessValue("0", brightness));
                assert(brightness == 0);
                assert(parseLedMatrixBrightnessValue("1", brightness));
                assert(brightness == 1);
                assert(parseLedMatrixBrightnessValue("48", brightness));
                assert(brightness == 48);
                assert(parseLedMatrixBrightnessValue("255", brightness));
                assert(brightness == 255);

                assert(!parseLedMatrixBrightnessValue("", brightness));
                assert(!parseLedMatrixBrightnessValue("-1", brightness));
                assert(!parseLedMatrixBrightnessValue("256", brightness));
                assert(!parseLedMatrixBrightnessValue("12x", brightness));

                LedMatrixEffectMode effectMode = LedMatrixEffectMode::kCatOnly;
                assert(parseLedMatrixEffectMode("CAT ONLY", effectMode));
                assert(effectMode == LedMatrixEffectMode::kCatOnly);
                assert(parseLedMatrixEffectMode("cat", effectMode));
                assert(effectMode == LedMatrixEffectMode::kCatOnly);
                assert(parseLedMatrixEffectMode("cat-only", effectMode));
                assert(effectMode == LedMatrixEffectMode::kCatOnly);
                assert(parseLedMatrixEffectMode("cat + marquee", effectMode));
                assert(effectMode == LedMatrixEffectMode::kCatMessagePlayback);
                assert(parseLedMatrixEffectMode("playback", effectMode));
                assert(effectMode == LedMatrixEffectMode::kCatMessagePlayback);
                assert(!parseLedMatrixEffectMode("unknown", effectMode));

                assert(formatLedMatrixHomeAssistantErrorSummary(0, "") == "<nenhum>");
                assert(formatLedMatrixHomeAssistantErrorSummary(503, "") == "HTTP 503");
                assert(formatLedMatrixHomeAssistantErrorSummary(200, "IncompleteInput") == "JSON IncompleteInput");
                assert(formatLedMatrixHomeAssistantErrorSummary(500, "InvalidInput") == "HTTP 500 / JSON InvalidInput");

                return 0;
            }
            '''
        ),
        encoding="utf-8",
    )

    compile_result = subprocess.run(
        [
            "g++",
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-Ilib/LedMatrixCore/src",
            str(source),
            "-o",
            str(binary),
        ],
        cwd=".",
        capture_output=True,
        text=True,
        check=False,
    )

    assert compile_result.returncode == 0, compile_result.stderr

    run_result = subprocess.run(
        [str(binary)],
        capture_output=True,
        text=True,
        check=False,
    )
    assert run_result.returncode == 0, run_result.stderr
