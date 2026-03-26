# Inspiracoes e Possibilidades

## O que ja faz sentido neste formato 8x8

- Letreiro curto com mensagens objetivas
- Tarefas prioritarias do dia
- Lembretes puxados de notas
- Mascote animado para estado ocioso
- Indicadores simples de contexto, como `FOCO`, `CALL`, `BUILD`, `DONE`

## Integracoes uteis

### Kanban

Em vez de tentar falar com a API do board direto do microcontrolador, o caminho mais limpo e:

1. um script no computador consulta o board
2. converte os itens em frases curtas
3. envia por serial com `TEXT:...` ou atualiza `config.yaml`

Boas fontes para esse caminho:

- GitHub Projects v2 via GraphQL:
  `https://docs.github.com/en/graphql/reference/objects#projectv2`
- Trello REST API:
  `https://developer.atlassian.com/cloud/trello/rest/`

### Obsidian

O plugin Local REST API permite ler e escrever notas localmente via HTTP autenticado, o que encaixa bem com um script que gere mensagens para a matriz.

Referencia:

- `https://github.com/coddingtonbear/obsidian-local-rest-api`

## Referencias visuais e tecnicas

- Adafruit Matrix Portal New Guide Scroller
  `https://learn.adafruit.com/matrix-portal-new-guide-scroller?view=all`
  Bom exemplo de feed textual rolando e de como dados externos podem virar letreiro.

- WLED 2D effects
  `https://kno.wled.ge/features/effects/`
  Bom repertorio de ideias para animacoes pequenas, transicoes, efeitos de idle e estados.

- Fontino
  `https://github.com/rene-d/fontino`
  Referencia util para tipografia bitmap 8x8 em projetos Arduino/matriz.

## Ideias concretas para este repo

### 1. Letreiro de tarefa atual

Exibir so a tarefa em andamento:

- `KANBAN: REVISAR PR`
- `HOJE: PAGAR FATURA`
- `OBSIDIAN: FECHAR DAILY NOTE`

### 2. Fila curta de mensagens

Rotacionar 3 a 5 mensagens pequenas, com cor por contexto:

- azul para planejamento
- laranja para foco
- roxo para notas

### 3. Gato como feedback de estado

Usar o gato para comunicar:

- idle
- carregando
- concluido
- erro

Isso pode evoluir para sprites diferentes ou pequenas emocoes.

### 4. Atualizacao sem reflash

Como o firmware aceita comandos via Serial, um script local pode atualizar a matriz sem recompilar. Esse e provavelmente o caminho mais util no curto prazo.

## Observacao de produto

Em um display 8x8, "menos e melhor". O valor nao esta em mostrar muito texto, e sim em mostrar a coisa certa, na hora certa, com uma estetica memoravel.
