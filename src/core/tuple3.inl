//-----------------------------------------------------------------------------
template <typename T>
Tuple3<T>::Tuple3():
x_(0), y_(0), z_(0)
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
Tuple3<T>::Tuple3(T x, T y, T z):
x_(x), y_(y), z_(z)
{
    // empty
}

//-----------------------------------------------------------------------------
template <typename T>
void Tuple3<T>::clear()
{
    x_ = y_ = z_ = 0;
}


//-----------------------------------------------------------------------------
template <typename T>
void Tuple3<T>::set(T x, T y, T z)
{
    x_ = x;
    y_ = y;
    z_ = z;
}


//-----------------------------------------------------------------------------
// template <typename T>
// Tuple3<T> operator + (const Tuple3<T>& t1, const Tuple3<T>& t2)
// {   
//     return Tuple3<T>(t1.x_ + t2.x_, t1.y_ + t2.y_, t1.z_ + t2.z_);
// }


//-----------------------------------------------------------------------------
// template <typename T>
// Tuple3<T> operator - (const Tuple3<T>& t1, const Tuple3<T>& t2)
// {
//     return Tuple3<T>(t1.x_ - t2.x_, t1.y_ - t2.y_, t1.z_ - t2.z_);
// }
