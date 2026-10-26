# RefPro SILVER (VST3 · Windows · REAPER)

Plugin de análise de equilíbrio tonal inspirado no iZotope Tonal Balance Control 3.
O áudio passa intacto; o plugin só mede.

## Recursos
- Interface escura moderna (painel de alvos, gráfico com regiões Low / Low-Mid / High-Mid / High, medidores embaixo)
- Curva de referência **All-Purpose** com faixa de tolerância
- Medidores de Análise: **Vocal Balance**, **Dynamics**, **Stereo Width**
- **Equalizador de 10 bandas** (Bell, Low/High Shelf, Low/High Cut, Notch): dois cliques no gráfico criam uma banda, arraste o nó (freq/ganho), roda do mouse muda o Q, clique direito abre o menu. Botão *Equalizer* liga/desliga; na engrenagem, escolha analisar Pre-EQ ou Post-EQ
- **EQ dinâmico automático (botão Dynamic)**: 31 bandas de 1/3 de oitava que atenuam só o que passa da faixa de tolerância da curva de referência. Controles: Amount (%), Max Cut (dB), Tolerance (dB extra), Speed (Fast/Medium/Slow). O display mostra a curva do corte (laranja) e uma faixa "Dynamic cut" com os dB cortados em cada banda. Os limites de frequência de atuação (Dynamic Low/High Limit) ficam nos parâmetros do plugin no REAPER. Ordem do sinal: EQ estático → EQ dinâmico
- **Vocal Balance** heurístico (sem IA): considera só o que está no centro estéreo, com espectro tonal e modulação silábica (1,2-8 Hz), comparando a voz com o resto da mix. Mostra "No vocal detected" quando não encontra voz
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
- Vocal Balance, Dynamics e Stereo Width são estimativas próprias, não os algoritmos da iZotope. Vocal Balance: nível da voz (centro + tonal + silábico) vs. resto da mix; Dynamics: pico ÷ RMS de 3 s; Stereo Width: energia Side ÷ Mid.
- Limite do Vocal Balance: instrumento tonal exatamente no centro com notas bem separadas pode ser confundido com voz. O alvo padrão (-6 dB) é ajustável em `Source/Targets.h` (`vocalDb`); o **Capture** grava o valor medido numa faixa de referência.
- O EQ dinâmico usa um detector próprio (média de 1,5 / 3 / 6 s) que mede o sinal antes do corte, para não corrigir a própria correção. Sem latência. Se soar "vivo" demais ou "achatado", ajuste Amount e Max Cut.
- O EQ, o EQ dinâmico e o novo Vocal Balance foram testados só com sinais sintéticos e compilação no Linux; teste no REAPER antes de usar em trabalho final.
