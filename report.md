# BIBLIOTECA DE THREADS - RELATÓRIO

1. Termo de compromisso

Os membros do grupo afirmam que todo o código desenvolvido para este
trabalho é de autoria própria.  Exceto pelo material listado no item
3 deste relatório, os membros do grupo afirmam não ter copiado
material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço

Preencha as linhas abaixo com o nome e o e-mail dos integrantes do
grupo.  Substitua marcadores `XX` pela contribuição de cada membro
do grupo no desenvolvimento do trabalho (os valores devem somar
100%).

  * Nome pmoutinho@ufmg.br 50%
  * Nome samuelfantini@ufmg.br 50%

3. Referências bibliográficas
  * https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html
  * https://pubs.opengroup.org/onlinepubs/7908799/xsh/sigprocmask.html
  * https://linuxhint.com/c-sigprocmask-function-usage/
  * https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html
  

4. Estruturas de dados

  1. Descreva e justifique as estruturas de dados utilizadas para
     gerência das threads de espaço do usuário (partes 1, 2 e 5).

  * Parte 1: 
  - Foi criada a lista `threads_prontas` do tipo `dlist`
  para armazenar as `dccthreads` que estão prontas para execução.
  Desse modo, ao executar o `dccthread_init` primeiro iniciamos as
  variaveis necessarioas e em seguida criamos a thread gerente e a
  thread principal.  Em seguida por meio do `setcontext` colocamos o
  gerenciador que busca na lista `threads_prontas` qual a próxima
  thread a ser executada. Quando o gerenciador retoma o controle 
  ele remove a thread atual da lista de prontos e busca a próxima 
  thread.  

  - Nesse sentido, a função `dccthread_yield` teve somente a função
  de adicionar a thread atual para o final da lista de prontos e
  repassar o controle ao gerenciador.

  * Parte 2: 
  - Adicionamos ao struct `dccthread` duas propriedades
  novas para controlar threads que estão *aguardando* e threads que
  são *aguardadas* por outras threads. Além disso, criamos uma 
  lista `threads_aguardando` que contem as threads que não estão
  prontas para execução por aguardar a finalização de outra thread.
  Criamos também uma lista de `threads_finalizadas` na qual uma 
  quando uma thread é finalizada é adicionada para que quando
  `dccthread_wait` é chamada não ocorra problemas como esperar uma 
  thread que já foi finalizada.  

  - Dessa forma, quando `dccthread_wait` é chamado, caso a thread 
  a ser aguardada não tenha sido finalizada a thread atual é 
  adicionada a lista de `threads_aguardando` e removida da lista
  `threads_prontas`. Além disso, é preenchida a propriedade
  `thread_aguardada` indicando qual thread a thread colocada em 
  espera está aguardando. Caso a thread a ser aguardada já tenha
  sido finalizada a execução da thread atual segue normalmente.

  - Por fim, para concluir o que foi solicitado na parte 2
  implementamos a função `dccthread_exit`. Essa função busca na 
  lista de `threads_agurdadando` se ela é esperada por alguma
  thread. A função `verifica_thread_aguardada` foi criada para 
  auxiliar nesse processo. Caso encontre alguma thread aguardando
  ele adiciona essa thread novamente a lista de threads prontas. 

  * Parte 5:

  2. Descreva o mecanismo utilizado para sincronizar chamadas de
     dccthread_yield e disparos do temporizador (parte 4).

  * Parte 4: Basciamente foi adicionado um bloqueador do sinal 
  gerado pelo disparo do temporizador ( SIGRTMIN ). Desse modo, 
  quando o `dccthread_yield` está sendo executado os disparos do 
  temporizados são interrompidos evitando as condições de corrida.
  Esse bloqueio também foi adicionado a demais pontos onde a condição
  de corrida poderia existir. 

Outros pontos implementados: 
Realizamos a implementação de dois métodos sugeridos no TP
`dccthread_nwaiting`: Que retorna a quantidade de threads na lista
de threads aguardando
`dccthread_nexited`: Que percorre a lista de threads finalizadas e vê
quantas dela não eram aguardadas por ninguém
