// Adiciona isso no HTML do jogo depois Carlos <script src="sistemaMapa.js"></script>

// E isso aqui dentro do gameLoop:
// if(mapaAberto){

//     updateMapa();
//     desenharMapa(ctx, canvas);

//     requestAnimationFrame(gameLoop);
//     return;
// }


function criarMiniMapa(ctx, canvas){

    // =========================
    // VARIÁVEIS
    // =========================

    let mapaAberto = false;
    let faseSelecionada = 0;

    const fases = [
        { x: 240, y: 380 },
        { x: 610, y: 380 },
        { x: 980, y: 380 }
    ];

    let quadradoX = fases[0].x;
    let quadradoY = fases[0].y;

    let destinoX = fases[0].x;
    let destinoY = fases[0].y;

    // menor = mais suave
    // maior = mais rápido
    const velocidadeMapa = 0.08;
    let movendoNoMapa = false;

    const imagemMapa = new Image();
    imagemMapa.src = "mapaImagem.jpg";

    // =========================
    // CONTROLES
    // =========================

    document.addEventListener("keydown", (e) => {

        if(!mapaAberto) return;
        if(movendoNoMapa) return;

        // esquerda
        if((e.key === "a" || e.key === "A") && faseSelecionada > 0){

            faseSelecionada--;

            destinoX = fases[faseSelecionada].x;
            destinoY = fases[faseSelecionada].y;

            movendoNoMapa = true;
        }

        // direita
        if((e.key === "d" || e.key === "D") && faseSelecionada < 2){

            faseSelecionada++;

            destinoX = fases[faseSelecionada].x;
            destinoY = fases[faseSelecionada].y;

            movendoNoMapa = true;
        }

        // entrar na fase
        if(e.key === "Enter"){

            iniciarFase(faseSelecionada + 1);
        }

        });


    // =========================
    // MOVIMENTO
    // =========================

    function updateMapa(){

        // pega a distância entre o quadrado e o destino
        // e move apenas uma parte dela por frame

        // se estiver longe anda rápido
        // se estiver perto desacelera sozinho
        quadradoX += (destinoX - quadradoX) * velocidadeMapa;
        quadradoY += (destinoY - quadradoY) * velocidadeMapa;

        if(
            // Math.abs remove números negativos
            // ajuda a medir a distância real
            Math.abs(quadradoX - destinoX) < 1 &&
            Math.abs(quadradoY - destinoY) < 1
        ){
            quadradoX = destinoX;
            quadradoY = destinoY;

            movendoNoMapa = false;
        }
    }


    // =========================
    // MOSTRAR MAPA
    // =========================

    function desenharMapa(){

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.drawImage(
            imagemMapa,
            0,
            0,
            canvas.width,
            canvas.height
        );

        ctx.fillStyle = "red";

        ctx.fillRect(
            quadradoX,
            quadradoY,
            40,
            40
        );

        ctx.fillStyle = "white";
        ctx.font = "28px Arial";

        ctx.fillText(
            "A/D = mover | ENTER = selecionar fase",
            300,
            660
        );
    }
// =========================
    // LOOP DO MAPA
    // =========================

    function loopMapa(){

        if(!mapaAberto) return;

        updateMapa();
        desenharMapa();

        // repete o loop infinitamente
        // criando a animação
        requestAnimationFrame(loopMapa);
    }


    // =========================
    // FUNÇÕES 
    // =========================

    function abrirMapa(){

        mapaAberto = true;

        loopMapa();
    }

    function fecharMapa(){

        mapaAberto = false;
    }

    function iniciarFase(numeroDaFase){

        console.log("Entrando na fase " + numeroDaFase);

        fecharMapa();

    }

    //daqui pra baixo entra o código do jogo

    // aqui voce coloca no fim do código
    // retorna funções para usar no jogo
    return {
        abrirMapa,
        fecharMapa,
        iniciarFase
    };

};

//Tenta chamar a função abrir mapa para abrir o mapa