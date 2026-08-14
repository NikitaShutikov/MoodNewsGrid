document.addEventListener("DOMContentLoaded", () => {
  const grid = document.getElementById("newsGrid");
  const modal = new bootstrap.Modal(document.getElementById("newsModal"));
  const modalTitle = document.getElementById("modalTitle");
  const originalText = document.getElementById("originalText");
  const rewrittenText = document.getElementById("rewrittenText");
  const moodSelect = document.getElementById("moodSelect");
  const generateBtn = document.getElementById("generateBtn");
  const spinner = document.getElementById("spinner");
  const errorAlert = document.getElementById("errorAlert");
  const modalSource = document.getElementById("modalSource");

  let currentNewsId = null;

  // Загрузка списка новостей
  async function loadNews() {
    try {
      const res = await fetch("/news");
      if (!res.ok) throw new Error("Не удалось загрузить новости");
      const newsList = await res.json();
      renderNews(newsList);
    } catch (err) {
      grid.innerHTML = `<div class="alert alert-danger">Ошибка загрузки новостей: ${err.message}</div>`;
    }
  }

  // Рендеринг карточек
  function renderNews(newsList) {
    if (!newsList.length) {
      grid.innerHTML =
        '<div class="col-12 text-center">Новостей пока нет.</div>';
      return;
    }
    grid.innerHTML = newsList
      .map(
        (news) => `
            <div class="col-md-4 col-lg-3 mb-4">
                <div class="card news-card h-100" data-id="${news.id}">
                    <div class="card-body">
                        <h5 class="card-title">${escapeHtml(news.title)}</h5>
                        <p class="card-text">${escapeHtml(news.original_text).substring(0, 120)}...</p>
                    </div>
                </div>
            </div>
        `,
      )
      .join("");

    // Обработчики кликов по карточкам
    document.querySelectorAll(".news-card").forEach((card) => {
      card.addEventListener("click", () => {
        const id = card.dataset.id;
        openModal(id);
      });
    });
  }

  // Открытие модального окна
  async function openModal(id) {
    currentNewsId = id;
    // Сброс состояния
    modalTitle.textContent = "Загрузка...";
    originalText.textContent = "Загрузка...";
    rewrittenText.textContent = "Выберите настроение и нажмите «Сгенерировать»";
    modalSource.href = "#";
    errorAlert.style.display = "none";
    spinner.style.display = "none";
    generateBtn.disabled = false;

    try {
      const res = await fetch(`/news/${id}`);
      if (!res.ok) {
        if (res.status === 404) throw new Error("Новость не найдена");
        throw new Error("Ошибка загрузки новости");
      }
      const data = await res.json();
      modalTitle.textContent = escapeHtml(data.title) || "Без заголовка";
      originalText.textContent = data.original_text || "Текст отсутствует";
      modalSource.href = data.source_url || "#";
      modalSource.textContent = data.source_url
        ? "Источник"
        : "Источник не указан";
      // Если уже есть text_with_mood (например, при повторном открытии с сохранённым настроением)
      if (data.text_with_mood) {
        rewrittenText.textContent = data.text_with_mood;
      }
      modal.show();
    } catch (err) {
      errorAlert.textContent = "Ошибка: " + err.message;
      errorAlert.style.display = "block";
      modal.show();
    }
  }

  // Генерация переписанного текста
  async function generateRewritten() {
    if (!currentNewsId) return;
    const mood = moodSelect.value;
    if (!mood) return;

    // Показать спиннер, отключить кнопку
    spinner.style.display = "inline-block";
    generateBtn.disabled = true;
    errorAlert.style.display = "none";
    rewrittenText.textContent = "Генерация...";

    try {
      const res = await fetch(`/news/${currentNewsId}?mood=${mood}`);
      if (!res.ok) {
        if (res.status === 400) throw new Error("Недопустимое настроение");
        if (res.status === 404) throw new Error("Новость не найдена");
        throw new Error("Ошибка сервера");
      }
      const data = await res.json();
      if (data.text_with_mood) {
        rewrittenText.textContent = data.text_with_mood;
      } else {
        rewrittenText.textContent = "Не удалось получить переписанный текст.";
      }
    } catch (err) {
      errorAlert.textContent = "Ошибка: " + err.message;
      errorAlert.style.display = "block";
      rewrittenText.textContent = "Ошибка генерации. Попробуйте снова.";
    } finally {
      spinner.style.display = "none";
      generateBtn.disabled = false;
    }
  }

  // Вспомогательная функция для экранирования HTML
  function escapeHtml(text) {
    if (!text) return "";
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
  }

  // Обработчик кнопки "Сгенерировать"
  generateBtn.addEventListener("click", generateRewritten);

  // Загружаем новости при старте
  loadNews();
});
