import subprocess
import textwrap


def test_led_matrix_message_priority_prefers_serial_over_home_assistant_and_config(tmp_path):
    source = tmp_path / "message_priority_test.cpp"
    binary = tmp_path / "message_priority_test"
    source.write_text(
        textwrap.dedent(
            r'''
            #include <cassert>
            #include <cstdint>
            #include <cstring>
            #include <LedMatrixMessagePriority.h>

            int main() {
                assert(resolveLedMatrixMessageSource(false, false) == LedMatrixMessageSource::Config);
                assert(resolveLedMatrixMessageSource(false, true) == LedMatrixMessageSource::HomeAssistant);
                assert(resolveLedMatrixMessageSource(true, false) == LedMatrixMessageSource::Serial);
                assert(resolveLedMatrixMessageSource(true, true) == LedMatrixMessageSource::Serial);

                assert(std::strcmp(ledMatrixMessageSourceLabel(LedMatrixMessageSource::Config), "CONFIG") == 0);
                assert(std::strcmp(ledMatrixMessageSourceLabel(LedMatrixMessageSource::HomeAssistant), "HOME_ASSISTANT") == 0);
                assert(std::strcmp(ledMatrixMessageSourceLabel(LedMatrixMessageSource::Serial), "SERIAL") == 0);

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
