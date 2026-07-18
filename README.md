# Festa na República

Simulação em C com pthreads que modela uma república estudantil: moradores circulam entre uma área comum e os quartos, um zelador faz rondas periódicas e despeja quartos "flagrados" em festa. O objetivo é exercitar sincronização com **mutex + variáveis de condição + semáforos**, evitando data races e deadlocks.

Feito para a disciplina MC504 (Sistemas Operacionais) - Unicamp.

## Como funciona

- **Moradores** (`resident.c`): cada um roda em sua própria thread e alterna entre "pensar" na área comum e tentar entrar em um quarto aleatório. Ao entrar, permanece por um tempo de festa (ou até ser despejado) e depois sai.
- **Quartos** (`room.c`): controlam capacidade física via semáforo, e o estado (ocupantes, inspeção, despejo) via mutex + variável de condição.
- **Zelador** (`zelador.c`): thread que inspeciona os quartos em rodízio; se o número de ocupantes atingir o limiar de "festa", despeja todos os moradores do quarto.
- **Renderizador** (`render.c`): thread que desenha no terminal o estado atual de todos os quartos e o log de eventos mais recentes.
- **Log de eventos** (`eventlog.c`): buffer circular thread-safe com as últimas ocorrências (entradas, saídas, flagrantes, despejos).

`Ctrl+C` interrompe a simulação de forma limpa (todas as threads são sinalizadas e finalizadas com `pthread_join`).

## Build

```sh
make        # build otimizado (-O2)
make debug  # build com AddressSanitizer + UndefinedBehaviorSanitizer
make tsan   # build com ThreadSanitizer (detecção de data races)
make clean  # remove objetos e binário
```

## Uso

```sh
./republica [opções]
make run    # equivalente a: make all && ./republica
```

| Opção | Descrição | Padrão |
|---|---|---|
| `-r, --rooms N` | número de quartos | 5 |
| `-m, --residents N` | número de moradores | 12 |
| `-c, --capacity N` | capacidade física por quarto | 4 |
| `-t, --threshold N` | ocupantes mínimos para "festa ativa" | 3 |
| `-i, --inspect-ms N` | intervalo entre inspeções do zelador | 1500 |
| `--think-min-ms` / `--think-max-ms` | tempo min/max na área comum | 500 / 2000 |
| `--party-min-ms` / `--party-max-ms` | tempo min/max de festa | 2000 / 6000 |
| `-d, --duration-s N` | duração da simulação; `0` = até `Ctrl+C` | 60 |
| `--frame-ms N` | intervalo de redesenho do visualizador | 200 |
| `-s, --seed N` | semente aleatória | baseada no relógio |
| `-h, --help` | mostra a mensagem de ajuda | |

## Estrutura do código

```
src/
├── main.c       # ponto de entrada, cria/junta threads, trata SIGINT
├── config.*     # parsing de argumentos e valores padrão
├── house.*      # estado global compartilhado (quartos, log, contadores)
├── room.*       # sincronização por quarto (mutex, cond var, semáforo)
├── resident.*   # thread de morador
├── zelador.*    # thread do zelador (inspeção/despejo)
├── render.*     # thread de visualização no terminal
├── eventlog.*   # log de eventos circular thread-safe
└── util.*       # helpers (sleep interrompível, tempo, aleatoriedade)
```

## Autores

- Felipe Rocha Verol - RA: 248552
- Henrique Cazarim Meirelles Alves - RA: 244763
- Luiz Felipe Lenharo - RA: 237896
- Theo Maceres Silva - RA: 220825
