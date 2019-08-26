프로젝트명: 비트코인 예측
개발 기간: 2019. 08. 14 (수) ~  2019. 00. 00 (-)[-일]
개발 목적: LSTM으로 비트코인을 예측해보자
개발자: 병장 이현
사용 기술: LSTM
개발 언어: C++, Eigen, Bithumb API

배운점:
 - Eigen을 이용하여 C++로 신경망(LSTM)을 개발했다.
 - Bithumb API를 사용했다.

보완할점:
 
TODO:
 - save bid total, ask total, bid unit, ask unit, bid average, ask average of transaction history for each minute
 - save bid total, ask total, bit unit, ask unit, bid average, ask average of order book for each minute
 - predict bid hightly price of transaction history after n minutes
 
 - maybe to better reward, use relative value (no absolute value)