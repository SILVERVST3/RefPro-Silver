# RefPro SILVER (VST3 · Windows · REAPER)

Plugin de análise de equilíbrio tonal inspirado no iZotope Tonal Balance Control 3.
O áudio passa intacto; o plugin só mede.

## Recursos
- Interface escura moderna (painel de alvos, gráfico com regiões Low / Low-Mid / High-Mid / High, medidores embaixo)
- Curva de referência **All-Purpose** com faixa de tolerância
- Medidores de Análise: **Vocal Balance**, **Dynamics**, **Stereo Width**
- **Capture**: salva a curva média atual como novo alvo (até 8, gravados no projeto)
- Visualização em curva ou barras, médias de 3 s / 10 s / 30 s / música inteira (menu da engrenagem)
- Extra Meters: curva rápida + valores numéricos em dB

## Compilar no GitHub
1. Crie um repositório e envie todo o conteúdo desta pasta (inclusive `.github/`).
2. Abra a aba **Actions** → *Build RefPro SILVER* (roda a cada push, ou use *Run workflow*).
3. Ao terminar, baixe o artefato **RefPro-SILVER-VST3-Windows**.

## Instalar no REAPER
1. Copie a pasta `RefPro SILVER.vst3` para `C:\Program Files\Common Files\VST3\`.
2. REAPER → Options → Preferences → Plug-ins → VST → **Re-scan**.
3. Coloque o plugin no **Master** (ou no canal a analisar), por último na cadeia.

## Ajustes
- Curva All-Purpose: `Source/Targets.h` (pontos freq / dB / tolerância).
- Alvos de Dynamics (PSR) e Stereo Width: campos `psr` e `widthDb` do mesmo arquivo.

## Observações
- A curva All-Purpose é uma aproximação original, não são dados da iZotope. Para referências reais, toque uma faixa que você admira e use **Capture**.
- Não inclui: EQ embutido, app standalone, Target Blender, controle remoto de outros EQs.
- Vocal Balance, Dynamics e Stereo Width são estimativas próprias (energia 300 Hz–3,4 kHz / total; pico ÷ RMS de 3 s; energia Side ÷ Mid), não os algoritmos da iZotope.
