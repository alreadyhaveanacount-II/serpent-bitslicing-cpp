# Serpent

Uma implementação de alto desempenho do Serpent usando fatiamento de bits para performance extrema

---
## Compilação

### Windows (MSVC/PowerShell)
```
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Linux

```
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
ninja
```

> **Nota para usuários Windows:** Certifique-se de que `clang++` e `ninja` estejam no seu PATH. Este projeto utiliza instruções AVX2 através do vetorizador SLP do Clang para alcançar 3.4 CPB.
> **Se você estiver no Linux:** Se o `clang++` e o `ninja` não estiverem instalados, use o comando: `sudo apt update && sudo apt install clang ninja-build`


---

## Características

- **Cifra de Bloco Serpent**: Implementação completa de 32 rodadas otimizada usando **Fatiamento de Bits**.
- **Operação em Modo CTR**: Implementação de cifra de fluxo de alto desempenho permitindo paralelismo massivo.
- **S-Boxes Fatiadas por Bits**: Utiliza sequências lógicas otimizadas de Dag Arne Osvik para máxima eficiência.
- **Benchmarking de Alta Precisão**: Acompanhamento de performance usando Ciclos Por Byte (CPB) via `RDTSCP` e serialização `LFENCE`.
- **Análise Detalhada de Taxa de Transferência**: Métricas em MB/s com cenários Médio, Melhor e Pior caso.
- **Análise Estatística**: Inclui Intervalo Interquartil (IIQ) para filtrar ruído do sistema e interferência do SO.
- **Arquitetura Moderna C++20**:
    - Manipulação de dados type-safe usando `std::span`.
    - Design orientado a objetos com API limpa e desacoplada.
- **Gerenciamento Seguro de Memória**: Uso estratégico de zeragem de memória e acesso direto a buffers para performance.

---

## Notas de Segurança

- **Educacional / Experimental**: Não destinado para uso em produção.
- **Validado com PractRand**: Testado com mais de 1GB sem anomalias significativas (mais de 10.000 testes).
- **Tempo de execução constante**: A implementação evita qualquer desvio de fluxo (branching) ou padrão de acesso à memória que dependa de dados sensíveis.
- **Verificado contra vetores de teste**: Verificado contra [vetores de teste oficiais](https://biham.cs.technion.ac.il).


---

## Notas de Design

- **Fatiamento de Bits**: Processa múltiplos blocos simultaneamente através da transposição de dados em planos de bits para eficiência similar a SIMD.
- **Transformação Linear**: Otimizada para manter a difusão enquanto permanece no domínio do fatiamento de bits.

---

## Resultados de Performance

```
=======================================================
 Testes de performance do Serpent
=======================================================
[ TAXA DE TRANSFERÊNCIA ]
  Melhor: 828.6635MB/s
  Pior: 507.3309MB/s
  Média: 778.8970MB/s
  Amplitude: 321.3325MB/s
  IIQ: 26.3642MB/s

[ TEMPO (segundos) ]
  Menor: 0.0097s
  Maior: 0.0158s
  Média: 0.0104s
  Amplitude: 0.0061s
  IIQ: 0.0003s

[ EFICIÊNCIA ]
  CPB médio: 3.0974 c/B
=======================================================
```
