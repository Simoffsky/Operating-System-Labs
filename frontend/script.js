function toUnixTimestamp(dateString) {
    const date = new Date(dateString);
    return Math.floor(date.getTime() / 1000);
}

// Функция для получения данных и отображения графика
function fetchData() {
    const start = document.getElementById('start').value;
    const end = document.getElementById('end').value;

    // Проверка наличия введённых значений
    if (!start || !end) {
        alert("Пожалуйста, укажите начальную и конечную дату.");
        return;
    }

    // Преобразуем даты в Unix timestamp
    const startTimestamp = toUnixTimestamp(start);
    const endTimestamp = toUnixTimestamp(end);

    // Формируем URL для запроса
    const url = `http://localhost:8080/data?start=${startTimestamp}&end=${endTimestamp}`;

    // Отправка запроса на сервер
    fetch(url)
        .then(response => {
            if (!response.ok) {
                throw new Error('Ошибка при получении данных');
            }
            return response.json();
        })
        .then(data => {
            document.getElementById('average').textContent = `Средняя температура: ${data.average.toFixed(2)}°C`;

            const labels = data.measurements.map(item => new Date(item.timestamp * 1000).toLocaleTimeString());
            const values = data.measurements.map(item => item.value);

            const ctx = document.getElementById('temperatureChart').getContext('2d');
            const chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [{
                        label: 'Температура',
                        data: values,
                        borderColor: 'rgba(75, 192, 192, 1)',
                        fill: false
                    }]
                },
                options: {
                    responsive: true,
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Время'
                            }
                        },
                        y: {
                            title: {
                                display: true,
                                text: 'Температура (°C)'
                            },
                            min: 20,
                            max: 30
                        }
                    }
                }
            });
        })
        .catch(error => {
            alert('Ошибка: ' + error.message);
        });
}