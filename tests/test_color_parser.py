import subprocess
import textwrap


def test_led_matrix_color_parser_rejects_invalid_rgb_and_accepts_expected_formats(tmp_path):
    source = tmp_path / "color_parser_test.cpp"
    binary = tmp_path / "color_parser_test"
    source.write_text(
        textwrap.dedent(
            r'''
            #include <cassert>
            #include <cstdint>
            #include <LedMatrixColorParser.h>

            int main() {
                LedMatrixRgb color{};

                assert(parseLedMatrixRgbTriplet("255, 140,0", color));
                assert(color.red == 255);
                assert(color.green == 140);
                assert(color.blue == 0);

                assert(parseLedMatrixHexColor("#00FFA0", color));
                assert(color.red == 0);
                assert(color.green == 255);
                assert(color.blue == 160);

                assert(parseLedMatrixHomeAssistantColor("0,255,160", color));
                assert(color.red == 0);
                assert(color.green == 255);
                assert(color.blue == 160);

                assert(!parseLedMatrixRgbTriplet("256,0,0", color));
                assert(!parseLedMatrixRgbTriplet("abc,0,0", color));
                assert(!parseLedMatrixRgbTriplet("1,2,3,4", color));
                assert(!parseLedMatrixRgbTriplet("1,,3", color));
                assert(!parseLedMatrixHexColor("#00FFAG", color));
                assert(!parseLedMatrixHomeAssistantColor("unknown", color));
                assert(!parseLedMatrixHomeAssistantColor("unavailable", color));
                assert(!parseLedMatrixHomeAssistantColor("", color));

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
