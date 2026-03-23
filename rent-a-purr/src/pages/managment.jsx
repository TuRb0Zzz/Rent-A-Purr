import React, { useState, useEffect } from 'react';
import { Plus, Trash2, Edit3, PawPrint, Calendar, Phone, Mail, User, CheckCircle, ListTodo } from 'lucide-react';
import AddCat from '../components/addcat';
import EditCat from '../components/editcat';
import AddReq from '../components/addreq';
import EditReq from '../components/editreq';
// ИМПОРТИРУЕМ getImageUrl из API:
import { api, getImageUrl } from '../api';

// Защищенная утилита форматирования даты (не упадет, если придет undefined)
const formatDateTime = (dateString) => {
    if (!dateString) return 'Дата неизвестна';
    try {
        return new Intl.DateTimeFormat('ru-RU', {
            day: 'numeric', month: 'long', year: 'numeric', hour: '2-digit', minute: '2-digit'
        }).format(new Date(dateString.replace(' ', 'T')));
    } catch (e) {
        return dateString;
    }
};

export default function Management() {
    const [activeTab, setActiveTab] = useState('cats');
    const [cats, setCats] = useState([]);
    const [bookings, setBookings] = useState([]);
    const [loading, setLoading] = useState(true);

    const [isAddCatOpen, setIsAddCatOpen] = useState(false);
    const [catToEdit, setCatToEdit] = useState(null);
    const[isAddReqOpen, setIsAddReqOpen] = useState(false);
    const [reqToEdit, setReqToEdit] = useState(null);

    const loadData = async () => {
        setLoading(true);
        try {
            const [catsRes, bookingsRes] = await Promise.all([
                api.getCats(),
                api.getAdminBookings().catch(() => ({ bookings: [] }))
            ]);

            setCats(catsRes.cats || []);
            const fetchedBookings = bookingsRes.bookings || [];
            const mappedBookings = fetchedBookings.map(b => ({
                ...b,
                status: b.status !== undefined ? b.status : 1,
                email: b.email || 'Нет email'
            }));

            setBookings(mappedBookings);

        } catch (e) {
            console.error("Ошибка загрузки данных админки:", e);
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => { loadData(); },[]);

    // --- Обработчики Котов ---
    const handleAddCat = async (newCatData) => {
        try {
            const formData = new FormData();
            formData.append('name', newCatData.name);
            formData.append('description', newCatData.description);
            formData.append('breed', newCatData.breed);
            formData.append('age', newCatData.age);

            if (newCatData.tags?.length) {
                formData.append('tags', newCatData.tags.join(','));
            }

            // Добавляем ВСЕ файлы под одним ключом 'photos'
            if (newCatData.photos && newCatData.photos.length > 0) {
                newCatData.photos.forEach(file => {
                    formData.append('photos', file);
                });
            }

            // Форматируем медицинские данные кратно 4
            if (newCatData.medical && newCatData.medical.length > 0) {
                const medString = newCatData.medical.map(m => `${m.iconName || 'stethoscope'},${m.label},${m.color},${m.bg}`).join(',');
                formData.append('medical', medString);
            }

            await api.addCat(formData);
            setIsAddCatOpen(false);
            loadData();
        } catch (e) {
            alert("Ошибка: " + e.message);
        }
    };

    const handleUpdateCat = async (updatedCatData) => {
        try {
            // По спецификации PUT /cats/{id} принимает только tags и medical
            const dataToUpdate = {
                tags: (updatedCatData.tags ||[]).join(','),
            };
            if (updatedCatData.medical && updatedCatData.medical.length > 0) {
                dataToUpdate.medical = updatedCatData.medical.map(m => `${m.iconName || 'stethoscope'},${m.label},${m.color},${m.bg}`).join(',');
            }

            await api.updateCat(updatedCatData.id, dataToUpdate);
            setCatToEdit(null);
            loadData();
        } catch (e) {
            alert("Ошибка: " + e.message);
        }
    };

    const handleDeleteCat = async (catId) => {
        if (window.confirm("Вы уверены, что хотите удалить кота?")) {
            // Здесь в будущем будет api.deleteCat(catId)
            setCats(cats.filter(c => c.id !== catId));
        }
    };

    // --- Обработчики Аренд ---
    const handleAddBooking = async (newBookingData) => {
        try {
            await api.addAdminBooking({
                cat_id: newBookingData.catId,
                email: newBookingData.customer.email || 'admin_added@test.ru',
                start_time: newBookingData.time[0],
                end_time: newBookingData.time[1]
            });
            setIsAddReqOpen(false);
            loadData();
        } catch (e) {
            alert("Ошибка: " + e.message);
        }
    };

    const handleUpdateBooking = async (updatedBookingData) => {
        try {
            await api.deleteAdminBooking({ booking_id: updatedBookingData.id });
            await api.addAdminBooking({
                cat_id: updatedBookingData.catId,
                email: updatedBookingData.customer.email || 'admin_added@test.ru',
                start_time: updatedBookingData.time[0],
                end_time: updatedBookingData.time[1]
            });
            setReqToEdit(null);
            loadData();
        } catch (e) {
            alert("Ошибка: " + e.message);
        }
    };

    const handleConfirmBooking = async (bookingId) => {
        try {
            await api.confirmAdminBooking({ booking_id: bookingId });
            loadData();
        } catch (e) {
            alert("Ошибка: " + e.message);
        }
    };

    const handleDeleteBooking = async (bookingId) => {
        if (window.confirm("Удалить или отклонить аренду?")) {
            try {
                await api.deleteAdminBooking({ booking_id: bookingId });
                loadData();
            } catch (e) {
                alert("Ошибка: " + e.message);
            }
        }
    };

    if (loading) {
        return <div className="h-full flex items-center justify-center font-bold text-slate-400">Загрузка данных админки...</div>;
    }

    return (
        <div className="min-h-full bg-white text-slate-900 font-m3 pb-20 relative">
            <div className="w-full px-4 sm:px-6 lg:px-8 pt-6 pb-2 md:pt-8 max-w-[1600px] mx-auto relative z-10">
                <div className="flex w-full h-20 sm:h-24 md:h-32 lg:h-40 gap-2 sm:gap-3 md:gap-5">
                    <div className="w-5 sm:w-8 md:w-10 lg:w-14 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                    <div className="flex-1 bg-[#F6828C] rounded-full flex items-center justify-center px-4 sm:px-8 shadow-sm">
                        <h1 className="font-seymour text-2xl sm:text-4xl md:text-5xl lg:text-[4.5rem] text-white tracking-wider text-center pt-2">Админка</h1>
                    </div>
                    <div className="w-12 sm:w-20 md:w-28 lg:w-40 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                </div>
            </div>

            <div className="max-w-7xl mx-auto px-4 mt-6 relative z-10 flex flex-col sm:flex-row justify-between items-center gap-4">
                <div className="inline-flex bg-slate-100 p-1.5 rounded-[20px]">
                    <button onClick={() => setActiveTab('cats')} className={`flex items-center gap-2 px-6 py-3 rounded-2xl font-extrabold ${activeTab === 'cats' ? 'bg-white text-[#F6828C] shadow-sm' : 'text-slate-500 hover:text-slate-700'}`}>
                        <PawPrint size={20} /> Каталог
                    </button>
                    <button onClick={() => setActiveTab('bookings')} className={`flex items-center gap-2 px-6 py-3 rounded-2xl font-extrabold ${activeTab === 'bookings' ? 'bg-white text-[#7838F5] shadow-sm' : 'text-slate-500 hover:text-slate-700'}`}>
                        <ListTodo size={20} /> Запросы и Аренды
                        {bookings.filter(b => b.status === 0).length > 0 && <span className="bg-[#FF2B4A] text-white text-xs px-2 py-0.5 rounded-full ml-1">{bookings.filter(b => b.status === 0).length}</span>}
                    </button>
                </div>

                {activeTab === 'cats' && (
                    <button onClick={() => setIsAddCatOpen(true)} className="bg-[#F6828C] text-white px-6 py-3.5 rounded-2xl font-extrabold shadow-lg shadow-[#F6828C]/30 hover:bg-[#FF2B4A] transition-colors flex items-center gap-2">
                        <Plus size={22} strokeWidth={3} /><span>Новый кот</span>
                    </button>
                )}
                {activeTab === 'bookings' && (
                    <button onClick={() => setIsAddReqOpen(true)} className="bg-[#7838F5] text-white px-6 py-3.5 rounded-2xl font-extrabold shadow-lg shadow-[#7838F5]/30 hover:bg-[#6025D1] transition-colors flex items-center gap-2">
                        <Plus size={22} strokeWidth={3} /><span>Добавить аренду</span>
                    </button>
                )}
            </div>

            <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 relative z-10">

                {/* === Вкладка КОТЫ === */}
                {activeTab === 'cats' && (
                    <div className="animate-tab">
                        <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
                            {cats.map((cat) => (
                                <div key={cat.id} className="bg-white rounded-[32px] p-4 sm:p-6 shadow-[0_4px_20px_rgb(0,0,0,0.03)] border-2 border-slate-100 hover:border-[#F6828C]/30 transition-colors flex flex-col sm:flex-row gap-6">
                                    <div className="relative w-full sm:w-48 h-56 sm:h-auto flex-shrink-0">
                                        {/* ЗАЩИЩЕННАЯ КАРТИНКА */}
                                        <img
                                            src={getImageUrl(cat?.filenames)}
                                            alt={cat.name || 'Кот'}
                                            className="w-full h-full object-cover rounded-[24px]"
                                            onError={(e) => { e.target.src = 'https://via.placeholder.com/600x400?text=Файл+не+найден' }}
                                        />
                                        <div className="absolute top-3 right-3 bg-white/90 px-3 py-1.5 rounded-full text-xs font-extrabold">{cat.age || '?'}</div>
                                    </div>
                                    <div className="flex-1 flex flex-col min-w-0">
                                        <h3 className="text-2xl font-extrabold text-slate-800 tracking-tight truncate">{cat.name || 'Без имени'}</h3>
                                        <p className="text-slate-500 font-bold truncate mb-2">{cat.breed || 'Неизвестная порода'}</p>
                                        <p className="text-slate-600 font-medium text-sm mb-4 line-clamp-2">{cat.description || 'Нет описания'}</p>

                                        {/* ЗАЩИЩЕННЫЕ ТЕГИ */}
                                        <div className="flex flex-wrap gap-2 mb-4">
                                            {(cat.tags ||[]).map((tag, index) => (
                                                <span key={index} className="px-3 py-1 bg-slate-100 text-slate-600 text-xs font-bold rounded-xl">{tag}</span>
                                            ))}
                                        </div>

                                        <div className="mt-auto border-t-2 border-slate-100 pt-5 flex gap-3">
                                            <button onClick={() => setCatToEdit(cat)} className="flex-1 flex items-center justify-center gap-2 px-4 py-3.5 rounded-2xl bg-[#7838F5]/10 hover:bg-[#7838F5]/20 text-[#7838F5] font-extrabold transition-colors">
                                                <Edit3 size={18} /> Изменить
                                            </button>
                                            <button onClick={() => handleDeleteCat(cat.id)} className="flex-1 flex items-center justify-center gap-2 px-4 py-3.5 rounded-2xl bg-[#FF2B4A]/10 hover:bg-[#FF2B4A]/20 text-[#FF2B4A] font-extrabold transition-colors">
                                                <Trash2 size={18} /> Удалить
                                            </button>
                                        </div>
                                    </div>
                                </div>
                            ))}
                        </div>
                    </div>
                )}

                {/* === Вкладка АРЕНДЫ === */}
                {activeTab === 'bookings' && (
                    <div className="animate-tab space-y-10">

                        {/* ОЖИДАЮЩИЕ (status: 0) */}
                        {bookings.filter(b => b.status === 0).length > 0 && (
                            <div>
                                <h2 className="text-2xl font-extrabold mb-6 text-slate-800 px-2 flex items-center gap-3"><span className="w-4 h-4 rounded-full bg-[#FFB300] inline-block"></span>Новые запросы</h2>
                                <div className="grid grid-cols-1 gap-6">
                                    {bookings.filter(b => b.status === 0).map(booking => {
                                        const cat = cats.find(c => c.id === booking.cat_id);
                                        return (
                                            <div key={booking.id} className="bg-white rounded-[32px] p-6 shadow-sm border-2 border-[#FFB300]/30 flex flex-col xl:flex-row gap-6">
                                                <div className="flex items-center gap-4 xl:w-1/4">
                                                    {/* ЗАЩИЩЕННАЯ КАРТИНКА */}
                                                    <img
                                                        src={getImageUrl(cat?.filenames)}
                                                        className="w-20 h-20 rounded-[18px] object-cover"
                                                        alt="cat"
                                                        onError={(e) => { e.target.src = 'https://via.placeholder.com/150?text=Нет+фото' }}
                                                    />
                                                    <div><p className="text-xs font-bold text-slate-400 uppercase">Кот</p><h4 className="text-xl font-extrabold text-slate-800">{cat?.name || booking.cat_name || 'Неизвестно'}</h4></div>
                                                </div>
                                                <div className="flex-1 bg-slate-50 rounded-[20px] p-4 flex flex-col justify-center border border-slate-100">
                                                    <p className="flex items-center gap-2 text-slate-800 font-bold"><User size={16} className="text-[#00C4D1]"/> {booking.nickname}</p>
                                                    <p className="flex items-center gap-2 text-slate-600 font-medium text-sm"><Mail size={16} className="text-[#7838F5]"/> {booking.email}</p>
                                                </div>
                                                <div className="flex-1 flex flex-col justify-between">
                                                    <div className="mb-4">
                                                        <p className="flex items-center gap-2 text-slate-800 font-bold mb-1"><Calendar size={18} className="text-[#F6828C]"/> {formatDateTime(booking.start_time)}</p>
                                                        <p className="flex items-center gap-2 text-slate-500 font-medium text-sm ml-7">до {formatDateTime(booking.end_time)}</p>
                                                    </div>
                                                    <div className="flex gap-3 mt-auto">
                                                        <button onClick={() => handleConfirmBooking(booking.id)} className="flex-1 py-3 bg-[#00D26A] hover:bg-[#00A855] text-white rounded-xl font-extrabold flex justify-center items-center gap-2"><CheckCircle size={18} /> Принять</button>
                                                        <button onClick={() => handleDeleteBooking(booking.id)} className="flex-1 py-3 bg-slate-100 hover:bg-slate-200 text-slate-600 rounded-xl font-extrabold">Отклонить</button>
                                                    </div>
                                                </div>
                                            </div>
                                        )
                                    })}
                                </div>
                            </div>
                        )}

                        {/* ПОДТВЕРЖДЕННЫЕ (status: 1) */}
                        <div>
                            <h2 className="text-2xl font-extrabold mb-6 text-slate-800 px-2 flex items-center gap-3"><span className="w-4 h-4 rounded-full bg-[#00D26A] inline-block"></span>Подтвержденные аренды</h2>
                            <div className="grid grid-cols-1 gap-6">
                                {bookings.filter(b => b.status === 1).map(booking => {
                                    const cat = cats.find(c => c.id === booking.cat_id);
                                    return (
                                        <div key={booking.id} className="bg-white rounded-[32px] p-6 shadow-sm border-2 border-slate-100 flex flex-col xl:flex-row gap-6">
                                            <div className="flex items-center gap-4 xl:w-1/4">
                                                {/* ЗАЩИЩЕННАЯ КАРТИНКА */}
                                                <img
                                                    src={getImageUrl(cat?.filenames)}
                                                    className="w-20 h-20 rounded-[18px] object-cover"
                                                    alt="cat"
                                                    onError={(e) => { e.target.src = 'https://via.placeholder.com/150?text=Нет+фото' }}
                                                />
                                                <div><p className="text-xs font-bold text-slate-400 uppercase">Кот</p><h4 className="text-xl font-extrabold text-slate-800">{cat?.name || booking.cat_name || 'Неизвестно'}</h4></div>
                                            </div>
                                            <div className="flex-1 bg-slate-50 rounded-[20px] p-4 flex flex-col justify-center border border-slate-100">
                                                <p className="flex items-center gap-2 text-slate-800 font-bold"><User size={16} className="text-[#00C4D1]"/> {booking.nickname}</p>
                                                <p className="flex items-center gap-2 text-slate-600 font-medium text-sm"><Mail size={16} className="text-[#7838F5]"/> {booking.email}</p>
                                            </div>
                                            <div className="flex-1 flex flex-col justify-between">
                                                <div className="mb-4 bg-[#00D26A]/5 rounded-2xl p-4 border border-[#00D26A]/20">
                                                    <p className="flex items-center gap-2 text-[#00A855] font-extrabold mb-1"><Calendar size={18} /> {formatDateTime(booking.start_time)}</p>
                                                    <p className="flex items-center gap-2 text-slate-600 font-medium text-sm ml-7">до {formatDateTime(booking.end_time)}</p>
                                                </div>
                                                <div className="flex gap-3 mt-auto">
                                                    <button onClick={() => setReqToEdit(booking)} className="flex-1 py-3 bg-[#7838F5]/10 hover:bg-[#7838F5]/20 text-[#7838F5] rounded-xl font-extrabold flex justify-center items-center gap-2"><Edit3 size={18} /> Редакт.</button>
                                                    <button onClick={() => handleDeleteBooking(booking.id)} className="flex-1 py-3 bg-[#FF2B4A]/10 hover:bg-[#FF2B4A]/20 text-[#FF2B4A] rounded-xl font-extrabold">Отменить</button>
                                                </div>
                                            </div>
                                        </div>
                                    )
                                })}
                            </div>
                        </div>

                    </div>
                )}
            </main>

            {/* Модальные окна */}
            {isAddCatOpen && <AddCat onClose={() => setIsAddCatOpen(false)} onSubmit={handleAddCat} />}

            {catToEdit && (
                <EditCat
                    // Защищаем передаваемые пропсы (если форма EditCat ожидает image и tags)
                    cat={{
                        ...catToEdit,
                        image: catToEdit.filenames?.[0] || '',
                        tags: catToEdit.tags ||[]
                    }}
                    onClose={() => setCatToEdit(null)}
                    onSubmit={handleUpdateCat}
                />
            )}

            {isAddReqOpen && <AddReq cats={cats} onClose={() => setIsAddReqOpen(false)} onSubmit={handleAddBooking} />}

            {reqToEdit && (
                <EditReq
                    cats={cats}
                    booking={{
                        ...reqToEdit,
                        catId: reqToEdit.cat_id,
                        customer: { name: reqToEdit.nickname, phone: '', email: reqToEdit.email },
                        time:[reqToEdit.start_time, reqToEdit.end_time]
                    }}
                    onClose={() => setReqToEdit(null)}
                    onSubmit={handleUpdateBooking}
                />
            )}

        </div>
    );
}