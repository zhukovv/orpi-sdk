ORIENSE

Программа калибровки акселерометра и компаса не изменена. 
Код программы доступен по адресу https://github.com/Pansenti/linux-mpu9150

                            
				---Краткое описание---

Для корректного функционирования mpu9150 необходимо откалибровать его вручную.
Калибровка осуществляется с помощью программы imucal. Её необходимо запустить
дважды: один раз для акселерометра, и второй раз для компаса.

Калибровка акселерометра: 
1) запустить программу 
#	sudo [путь]/imucal -a
В результате выполнения в консоли появится следующая информация
#	X: min|current|max	Y: min|current|max	Z: min|current|max
здесь min, cur, max - соответственно, минимальное, текущее и среднее значения для каждой оси.
2) Необходимо медленно (!)/*чтобы на mpu воздействовало только ускорение свободного падения g*/
вращать mpu9150 вокруг всех осей до тех пор, пока значения min и max не перестанут меняться.
3) Когда значения min/max перестанут меняться, можно надать <Ctrl+C>, и программа сохранит результаты
калибровки акселерометра сохранятся в файле /home/pi/accelcal.txt /*можно легко изменить, см [1]*/

калибровка компаса:
1) запустить программу 
#	sudo [путь]/imucal -a
В результате выполнения в консоли появится следующая информация
#	X: min|current|max	Y: min|current|max	Z: min|current|max
здесь min, cur, max - соответственно, минимальное, текущее и среднее значения для каждой оси.
2) Необходимо вращать mpu9150 вокруг всех осей до тех пор, пока значения min и max не 
перестанут меняться.
3) Когда значения min/max перестанут меняться, можно надать <Ctrl+C>, и программа сохранит результаты
калибровки акселерометра сохранятся в файле /home/pi/magcal.txt /*можно легко изменить, см [1]*/

			------------------------------------------

Результаты калибрвоки, хранящиеся в файлах accelcal.txt и magcal.txt, используются в mpu9150_applied

[1] https://github.com/Pansenti/linux-mpu9150