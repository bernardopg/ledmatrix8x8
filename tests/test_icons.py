import subprocess
import textwrap


def test_led_matrix_icons_parse_aliases_and_expose_bitmaps(tmp_path):
    source = tmp_path / "icons_test.cpp"
    binary = tmp_path / "icons_test"
    source.write_text(
        textwrap.dedent(
            r"""
            #include <cassert>
            #include <cstdint>
            #include <cstring>
            #include <LedMatrixIcons.h>

            int main() {
                static_assert(LED_MATRIX_ICON_COUNT == 7);
                LedMatrixIconId icon = LedMatrixIconId::kError;

                assert(parseLedMatrixIconId("agenda", icon));
                assert(icon == LedMatrixIconId::kAgenda);
                assert(parseLedMatrixIconId("CALENDAR", icon));
                assert(icon == LedMatrixIconId::kAgenda);
                assert(parseLedMatrixIconId("tarefa", icon));
                assert(icon == LedMatrixIconId::kTask);
                assert(parseLedMatrixIconId("done", icon));
                assert(icon == LedMatrixIconId::kStatus);
                assert(parseLedMatrixIconId("FOCO", icon));
                assert(icon == LedMatrixIconId::kFocus);
                assert(parseLedMatrixIconId("phone", icon));
                assert(icon == LedMatrixIconId::kCall);
                assert(parseLedMatrixIconId("coffee", icon));
                assert(icon == LedMatrixIconId::kBreak);
                assert(parseLedMatrixIconId("erro", icon));
                assert(icon == LedMatrixIconId::kError);
                assert(!parseLedMatrixIconId("unknown", icon));

                const LedMatrixIconDefinition *status = ledMatrixIconById(LedMatrixIconId::kStatus);
                assert(status != nullptr);
                assert(std::strcmp(status->name, "status") == 0);
                assert(status->defaultRed == 0);
                assert(status->defaultGreen == 255);
                assert(status->defaultBlue == 160);
                assert(status->rows[0] == 0x3C);
                assert(std::strcmp(ledMatrixIconLabel(LedMatrixIconId::kFocus), "FOCO") == 0);
                assert(std::strstr(ledMatrixIconCommandList(), "agenda") != nullptr);

                return 0;
            }
            """
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
    run_result = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
    assert run_result.returncode == 0, run_result.stderr
